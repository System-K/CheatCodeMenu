#pragma once

namespace CheatCodeMenu {

class SystemMenuInjector {
public:
  static SystemMenuInjector *GetSingleton();
  void Register();
  bool InjectMenuEntry();
  void TryInjectWithRetry(int retryCount = 0);
  bool IsInjected() const { return m_injected; }
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

class CCMButtonCallback : public RE::GFxFunctionHandler {
public:
  void Call(Params &a_params) override;
};

} // namespace CheatCodeMenu
