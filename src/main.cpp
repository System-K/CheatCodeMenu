#include "ConfigManager.h"
#include "HotkeyManager.h"
#include "Serialization.h"
#include "SystemMenuInjector.h"
#include "cheat/CheatRegistry.h"
#include "cheat/CheatStateStore.h"
#include "papyrus/PapyrusAPI.h"

using namespace CheatCodeMenu;

static void InitializeCheatSystem() {
  // Load cheat definitions from JSON files
  CheatRegistry::GetSingleton()->LoadFromDisk();

  // Initialize runtime state from definitions
  CheatStateStore::GetSingleton()->Initialize();
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message *message) {
  switch (message->type) {
  case SKSE::MessagingInterface::kDataLoaded:
    // Load configuration
    HotkeyManager::GetSingleton()->LoadConfig();
    ConfigManager::GetSingleton()->LoadConfig();

    // Initialize cheat system
    InitializeCheatSystem();

    // Register hotkey input sink
    HotkeyManager::GetSingleton()->RegisterSink();

    // Register System menu injector for pause menu button
    SystemMenuInjector::GetSingleton()->Register();

    logger::info("CheatCodeMenu initialized successfully");
    break;
  }
}

extern "C" DLLEXPORT bool SKSEAPI
SKSEPlugin_Load(const SKSE::LoadInterface *a_skse) {
  REL::Module::reset();

  auto g_messaging = reinterpret_cast<SKSE::MessagingInterface *>(
      a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));

  if (!g_messaging) {
    logger::critical("Failed to load messaging interface! This error is fatal, "
                     "plugin will not load.");
    return false;
  }

  logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

  SKSE::Init(a_skse);
  SKSE::AllocTrampoline(1 << 10);

  // Register serialization callbacks
  Serialization::Register();

  // Register Papyrus native functions
  SKSE::GetPapyrusInterface()->Register(PapyrusAPI::Register);

  // Register for SKSE messages
  g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

  return true;
}
