#include "HotkeyManager.h"
#include <CLIBUtil/detail/SimpleIni.h>

namespace CheatCodeMenu {
HotkeyManager *HotkeyManager::GetSingleton() {
  static HotkeyManager singleton;
  return &singleton;
}

void HotkeyManager::LoadConfig() {
  CSimpleIniA ini;
  ini.SetUnicode();

  auto path = std::filesystem::path("Data/SKSE/Plugins/CheatCodeMenu.ini");

  if (ini.LoadFile(path.string().c_str()) == SI_OK) {
    const char *scanCodeStr = ini.GetValue("Hotkey", "ScanCode", "0x43");

    // Parse hex or decimal
    try {
      if (scanCodeStr[0] == '0' &&
          (scanCodeStr[1] == 'x' || scanCodeStr[1] == 'X')) {
        m_scanCode =
            static_cast<uint32_t>(std::stoul(scanCodeStr, nullptr, 16));
      } else {
        m_scanCode =
            static_cast<uint32_t>(std::stoul(scanCodeStr, nullptr, 10));
      }
    } catch (...) {
      logger::warn("Failed to parse ScanCode from INI, using default (F9)");
      m_scanCode = 0x43;
    }

    logger::info("Loaded hotkey scancode: 0x{:X}", m_scanCode);
  } else {
    logger::info("No INI file found, using default hotkey (F9 = 0x43)");
  }
}

void HotkeyManager::RegisterSink() {
  HotkeyEventSink::GetSingleton(); // Ensure singleton exists

  auto *inputDeviceManager = RE::BSInputDeviceManager::GetSingleton();
  if (inputDeviceManager) {
    inputDeviceManager->AddEventSink(HotkeyEventSink::GetSingleton());
    logger::info("Registered hotkey input event sink");
  } else {
    logger::error("Failed to get BSInputDeviceManager");
  }
}

// ============================================
// HotkeyEventSink
// ============================================

HotkeyEventSink *HotkeyEventSink::GetSingleton() {
  static HotkeyEventSink singleton;
  return &singleton;
}

RE::BSEventNotifyControl
HotkeyEventSink::ProcessEvent(RE::InputEvent *const *a_event,
                              RE::BSTEventSource<RE::InputEvent *> *) {
  if (!a_event) {
    return RE::BSEventNotifyControl::kContinue;
  }

  auto *hotkeyManager = HotkeyManager::GetSingleton();
  uint32_t targetScanCode = hotkeyManager->GetScanCode();

  for (auto *event = *a_event; event; event = event->next) {
    if (event->eventType != RE::INPUT_EVENT_TYPE::kButton) {
      continue;
    }

    auto *buttonEvent = static_cast<RE::ButtonEvent *>(event);

    // Only trigger on key down (not held or released)
    if (!buttonEvent->IsDown()) {
      continue;
    }

    // Check if this is our hotkey
    if (buttonEvent->GetIDCode() == targetScanCode) {
      // Check we're not in a menu that would conflict
      auto *ui = RE::UI::GetSingleton();
      if (ui && !ui->GameIsPaused()) {
        SendOpenMenuEvent();
      }
    }
  }

  return RE::BSEventNotifyControl::kContinue;
}

void HotkeyEventSink::SendOpenMenuEvent() {
  logger::info("Sending CheatCodeMenu_OpenMenu event");

  auto *eventSource = SKSE::GetModCallbackEventSource();
  if (eventSource) {
    SKSE::ModCallbackEvent event;
    event.eventName = "CheatCodeMenu_OpenMenu";
    event.strArg = "";
    event.numArg = 0.0f;
    event.sender = nullptr;

    eventSource->SendEvent(&event);
  } else {
    logger::error("Failed to get ModCallbackEventSource");
  }
}
} // namespace CheatCodeMenu
