#pragma once

#include <cstdint>
#include <optional>

namespace CheatCodeMenu {
class HotkeyManager {
public:
  static HotkeyManager *GetSingleton();

  void LoadConfig();

  void RegisterSink();

  std::optional<uint32_t> GetScanCode() const { return m_scanCode; }
  bool IsEnabled() const { return m_scanCode.has_value(); }

private:
  HotkeyManager() = default;
  ~HotkeyManager() = default;

  HotkeyManager(const HotkeyManager &) = delete;
  HotkeyManager(HotkeyManager &&) = delete;
  HotkeyManager &operator=(const HotkeyManager &) = delete;
  HotkeyManager &operator=(HotkeyManager &&) = delete;

  std::optional<uint32_t> m_scanCode = 0x43;
};

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
