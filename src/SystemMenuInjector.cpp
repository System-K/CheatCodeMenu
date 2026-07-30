#include "SystemMenuInjector.h"
#include "pch.h"
#include <CLIBUtil/detail/SimpleIni.h>

namespace CheatCodeMenu {

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
  if (m_injected)
    return true;

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

  RE::GFxValue systemPage;
  if (!movie->GetVariable(
          &systemPage, "_root.QuestJournalFader.Menu_mc.SystemFader.Page_mc")) {
    logger::warn("[CCM] SystemFader.Page_mc not available (System tab may not "
                 "be active)");
    return false;
  }

  if (systemPage.IsUndefined()) {
    logger::warn("[CCM] SystemPage is undefined");
    return false;
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

  std::uint32_t arraySize = entryList.GetArraySize();
  std::uint32_t insertIndex = arraySize;

  for (std::uint32_t i = 0; i < arraySize; ++i) {
    RE::GFxValue entry;
    if (entryList.GetElement(i, &entry)) {
      RE::GFxValue textValue;
      if (entry.GetMember("text", &textValue) && textValue.IsString()) {
        const char *text = textValue.GetString();
        if (text && std::strcmp(text, "$CONTROLS") == 0) {
          insertIndex = i + 1;
          logger::info("[CCM] Found $CONTROLS at index {}, will insert at {}",
                       i, insertIndex);
          break;
        }
      }
    }
  }

  RE::GFxValue newEntry;
  movie->CreateObject(&newEntry);

  std::string buttonTextStr = "CHEAT CODES";

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

  if (insertIndex < arraySize) {
    for (std::uint32_t i = arraySize; i > insertIndex; --i) {
      RE::GFxValue element;
      entryList.GetElement(i - 1, &element);
      entryList.SetElement(i, element);
    }
    entryList.SetElement(insertIndex, newEntry);
  } else {
    entryList.PushBack(newEntry);
  }

  auto *callbackHandler = new CCMButtonCallback();
  RE::GFxValue callbackFunc;
  movie->CreateFunction(&callbackFunc, callbackHandler);

  RE::GFxValue originalCallback;
  if (systemPage.GetMember("onCategoryButtonPress", &originalCallback))
    systemPage.SetMember("CCM_originalOnCategoryButtonPress", originalCallback);
  systemPage.SetMember("CCM_callback", callbackFunc);

  movie->SetVariable("_global.CCM_ButtonCallback", callbackFunc);
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

  logger::info("[CCM] Injection attempt {} failed, scheduling retry",
               retryCount + 1);
  auto *injector = this;
  int nextRetry = retryCount + 1;

  SKSE::GetTaskInterface()->AddTask(
      [injector, nextRetry]() { injector->TryInjectWithRetry(nextRetry); });
}

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
      logger::info("[CCM] Journal Menu opened");
      injector->TryInjectWithRetry(0);
    } else {
      injector->ResetInjection();
      logger::info("[CCM] Journal Menu closed, reset injection state");
    }
  }

  return RE::BSEventNotifyControl::kContinue;
}

void CCMButtonCallback::Call(Params & /*a_params*/) {
  logger::info("[CCM] Cheat Codes button pressed!");

  auto *ui = RE::UI::GetSingleton();
  if (ui) {
    if (auto *queue = RE::UIMessageQueue::GetSingleton()) {
      queue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide,
                        nullptr);
    }
  }

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
