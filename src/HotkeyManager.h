#pragma once

#include <cstdint>

namespace CheatCodeMenu {
class HotkeyManager {
public:
  static HotkeyManager *GetSingleton();

  // Load hotkey configuration from INI
  void LoadConfig();

  // Register input event sink
  void RegisterSink();

  // Get configured scancode
  uint32_t GetScanCode() const { return m_scanCode; }

private:
  HotkeyManager() = default;
  ~HotkeyManager() = default;

  HotkeyManager(const HotkeyManager &) = delete;
  HotkeyManager(HotkeyManager &&) = delete;
  HotkeyManager &operator=(const HotkeyManager &) = delete;
  HotkeyManager &operator=(HotkeyManager &&) = delete;

  uint32_t m_scanCode = 0x43; // Default: F9
};

// Input event sink for hotkey detection
class HotkeyEventSink : public RE::BSTEventSink<RE::InputEvent *> {
public:
  static HotkeyEventSink *GetSingleton();

  RE::BSEventNotifyControl
  ProcessEvent(RE::InputEvent *const *a_event,
               RE::BSTEventSource<RE::InputEvent *> *a_eventSource) override;

private:
  HotkeyEventSink() = default;
  ~HotkeyEventSink() override = default;

  void SendOpenMenuEvent();
};
} // namespace CheatCodeMenu
