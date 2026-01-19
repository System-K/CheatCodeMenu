#pragma once

namespace CheatCodeMenu {

/// @brief Injects a "Cheat Codes" button into the System page of the Journal
/// Menu. Uses a hybrid approach: C++ handles entry injection, SWF handles
/// button callback.
class SystemMenuInjector {
public:
  static SystemMenuInjector *GetSingleton();

  /// @brief Register menu event listener and callback handler
  void Register();

  /// @brief Inject the Cheat Codes entry into the System menu's CategoryList
  /// @return true if injection succeeded, false if should retry
  bool InjectMenuEntry();

  /// @brief Try injection with retry logic
  void TryInjectWithRetry(int retryCount = 0);

  /// @brief Check if we've already injected into the current menu session
  bool IsInjected() const { return m_injected; }

  /// @brief Reset injection state (called when menu closes)
  void ResetInjection() {
    m_injected = false;
    m_retryCount = 0;
  }

private:
  SystemMenuInjector() = default;
  ~SystemMenuInjector() = default;
  SystemMenuInjector(const SystemMenuInjector &) = delete;
  SystemMenuInjector &operator=(const SystemMenuInjector &) = delete;

  bool m_injected = false;
  int m_retryCount = 0;
  static constexpr int MAX_RETRIES = 10;
};

/// @brief Event sink for menu open/close events
class MenuEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
  static MenuEventSink *GetSingleton();

  RE::BSEventNotifyControl ProcessEvent(
      const RE::MenuOpenCloseEvent *a_event,
      RE::BSTEventSource<RE::MenuOpenCloseEvent> *a_eventSource) override;

private:
  MenuEventSink() = default;
  ~MenuEventSink() override = default;
};

/// @brief GFx callback handler for when the Cheat Codes button is pressed
class CCMButtonCallback : public RE::GFxFunctionHandler {
public:
  void Call(Params &a_params) override;
};

} // namespace CheatCodeMenu
