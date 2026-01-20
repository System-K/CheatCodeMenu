#include "SystemMenuInjector.h"
#include "pch.h"
#include <CLIBUtil/detail/SimpleIni.h>

namespace CheatCodeMenu {

// ============================================
// SystemMenuInjector
// ============================================

SystemMenuInjector *SystemMenuInjector::GetSingleton() {
  static SystemMenuInjector singleton;
  return &singleton;
}

void SystemMenuInjector::Register() {
  auto *ui = RE::UI::GetSingleton();
  if (ui) {
    ui->AddEventSink(MenuEventSink::GetSingleton());
    logger::info("[CCM] Registered menu event sink for System menu injection");
  } else {
    logger::error("[CCM] Failed to get UI singleton");
  }
}

bool SystemMenuInjector::InjectMenuEntry() {
  if (m_injected) {
    return true; // Already injected this session
  }

  auto *ui = RE::UI::GetSingleton();
  if (!ui) {
    logger::error("[CCM] UI singleton not available");
    return false;
  }

  auto *journalMenu = ui->GetMenu<RE::JournalMenu>().get();
  if (!journalMenu) {
    logger::error("[CCM] Journal Menu not found");
    return false;
  }

  auto *movie = journalMenu->uiMovie.get();
  if (!movie) {
    logger::error("[CCM] Journal Menu movie not available");
    return false;
  }

  // Navigate to the System page's CategoryList
  // Path:
  // _root.QuestJournalFader.Menu_mc.SystemFader.Page_mc.CategoryList_mc.List_mc
  RE::GFxValue systemPage;
  if (!movie->GetVariable(
          &systemPage, "_root.QuestJournalFader.Menu_mc.SystemFader.Page_mc")) {
    logger::warn("[CCM] SystemFader.Page_mc not available (System tab may not "
                 "be active)");
    return false; // Retry later
  }

  if (systemPage.IsUndefined()) {
    logger::warn("[CCM] SystemPage is undefined");
    return false; // Retry later
  }

  RE::GFxValue categoryListMc;
  if (!systemPage.GetMember("CategoryList_mc", &categoryListMc) ||
      categoryListMc.IsUndefined()) {
    logger::warn("[CCM] CategoryList_mc not found");
    return false;
  }

  RE::GFxValue listMc;
  if (!categoryListMc.GetMember("List_mc", &listMc) || listMc.IsUndefined()) {
    logger::warn("[CCM] List_mc not found");
    return false;
  }

  RE::GFxValue entryList;
  if (!listMc.GetMember("entryList", &entryList) || !entryList.IsArray()) {
    logger::warn("[CCM] entryList not found or not an array");
    return false;
  }

  // Find the index of $CONTROLS to insert after it
  std::uint32_t arraySize = entryList.GetArraySize();
  std::uint32_t insertIndex = arraySize; // Default to end

  for (std::uint32_t i = 0; i < arraySize; ++i) {
    RE::GFxValue entry;
    if (entryList.GetElement(i, &entry)) {
      RE::GFxValue textValue;
      if (entry.GetMember("text", &textValue) && textValue.IsString()) {
        const char *text = textValue.GetString();
        if (text && std::strcmp(text, "$CONTROLS") == 0) {
          insertIndex = i + 1; // Insert after CONTROLS
          logger::info("[CCM] Found $CONTROLS at index {}, will insert at {}",
                       i, insertIndex);
          break;
        }
      }
    }
  }

  // Create the new entry object
  RE::GFxValue newEntry;
  movie->CreateObject(&newEntry);

  // Get button text from INI (allows localization/customization)
  std::string buttonTextStr = "Cheat Codes"; // Default

  CSimpleIniA ini;
  ini.SetUnicode();
  auto iniPath = std::filesystem::path("Data/SKSE/Plugins/CheatCodeMenu.ini");
  if (ini.LoadFile(iniPath.string().c_str()) >= 0) {
    const char *iniValue = ini.GetValue("Menu", "ButtonText", nullptr);
    if (iniValue && iniValue[0] != '\0') {
      buttonTextStr = iniValue;
      logger::info("[CCM] Using INI button text: {}", buttonTextStr);
    }
  }

  RE::GFxValue textValue;
  movie->CreateString(&textValue, buttonTextStr.c_str());
  newEntry.SetMember("text", textValue);

  RE::GFxValue isCCMValue(true);
  newEntry.SetMember("isCCM", isCCMValue);

  // Insert the entry at the correct position
  // Since GFxValue doesn't have insert, we need to shift elements
  if (insertIndex < arraySize) {
    // Shift elements from insertIndex to end by 1
    for (std::uint32_t i = arraySize; i > insertIndex; --i) {
      RE::GFxValue element;
      entryList.GetElement(i - 1, &element);
      entryList.SetElement(i, element);
    }
    entryList.SetElement(insertIndex, newEntry);
  } else {
    // Just push to end
    entryList.PushBack(newEntry);
  }

  // Register callback for button press
  // We'll use a GFxFunctionHandler to intercept the button press
  auto *callbackHandler = new CCMButtonCallback();
  RE::GFxValue callbackFunc;
  movie->CreateFunction(&callbackFunc, callbackHandler);

  // Store original onCategoryButtonPress and replace with our wrapper
  RE::GFxValue originalCallback;
  if (systemPage.GetMember("onCategoryButtonPress", &originalCallback)) {
    // Store original for calling later
    systemPage.SetMember("CCM_originalOnCategoryButtonPress", originalCallback);
  }
  systemPage.SetMember("CCM_callback", callbackFunc);

  // Register the callback on the Flash global scope (SWF handles the actual
  // button press callback hooking)
  movie->SetVariable("_global.CCM_ButtonCallback", callbackFunc);

  // Call InvalidateData to refresh the list
  listMc.Invoke("InvalidateData", nullptr, nullptr, 0);

  m_injected = true;
  logger::info("[CCM] Successfully injected Cheat Codes button at index {}",
               insertIndex);
  return true;
}

void SystemMenuInjector::TryInjectWithRetry(int retryCount) {
  if (m_injected || retryCount >= MAX_RETRIES) {
    return;
  }

  m_retryCount = retryCount;

  if (InjectMenuEntry()) {
    logger::info("[CCM] Injection succeeded on attempt {}", retryCount + 1);
    return;
  }

  // Schedule retry with delay
  logger::info("[CCM] Injection attempt {} failed, scheduling retry",
               retryCount + 1);
  auto *injector = this;
  int nextRetry = retryCount + 1;

  SKSE::GetTaskInterface()->AddTask([injector, nextRetry]() {
    // Add a small delay by just scheduling another task
    injector->TryInjectWithRetry(nextRetry);
  });
}

// ============================================
// MenuEventSink
// ============================================

MenuEventSink *MenuEventSink::GetSingleton() {
  static MenuEventSink singleton;
  return &singleton;
}

RE::BSEventNotifyControl
MenuEventSink::ProcessEvent(const RE::MenuOpenCloseEvent *a_event,
                            RE::BSTEventSource<RE::MenuOpenCloseEvent> *) {
  if (!a_event) {
    return RE::BSEventNotifyControl::kContinue;
  }

  auto *injector = SystemMenuInjector::GetSingleton();

  if (a_event->menuName == RE::JournalMenu::MENU_NAME) {
    if (a_event->opening) {
      logger::info("[CCM] Journal Menu opened, will attempt injection");
      // Try injection with retry logic
      injector->TryInjectWithRetry(0);
    } else {
      // Menu closing - reset injection state
      injector->ResetInjection();
      logger::info("[CCM] Journal Menu closed, reset injection state");
    }
  }

  return RE::BSEventNotifyControl::kContinue;
}

// ============================================
// CCMButtonCallback
// ============================================

void CCMButtonCallback::Call(Params & /*a_params*/) {
  logger::info("[CCM] Cheat Codes button pressed!");

  // Close the Journal Menu
  auto *ui = RE::UI::GetSingleton();
  if (ui) {
    // Use UI message to close menu
    if (auto *queue = RE::UIMessageQueue::GetSingleton()) {
      queue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide,
                        nullptr);
    }
  }

  // Send the mod event to open the Cheat Code Menu
  // This is picked up by Papyrus to open the actual menu
  auto *eventSource = SKSE::GetModCallbackEventSource();
  if (eventSource) {
    SKSE::ModCallbackEvent event;
    event.eventName = "CheatCodeMenu_OpenMenu";
    event.strArg = "";
    event.numArg = 0.0f;
    event.sender = nullptr;
    eventSource->SendEvent(&event);
    logger::info("[CCM] Sent CheatCodeMenu_OpenMenu event");
  } else {
    logger::error("[CCM] Failed to get ModCallbackEventSource");
  }
}

} // namespace CheatCodeMenu
