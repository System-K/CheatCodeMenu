#include "ConfigManager.h"
#include "HotkeyManager.h"
#include "Serialization.h"
#include "SystemMenuInjector.h"
#include "cheat/CheatRegistry.h"
#include "cheat/CheatStateStore.h"
#include "papyrus/PapyrusAPI.h"

using namespace CheatCodeMenu;

static void InitializeCheatSystem() {
  CheatRegistry::GetSingleton()->LoadFromDisk();
  CheatStateStore::GetSingleton()->Initialize();
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message *message) {
  switch (message->type) {
  case SKSE::MessagingInterface::kDataLoaded:
    HotkeyManager::GetSingleton()->LoadConfig();
    ConfigManager::GetSingleton()->LoadConfig();

    InitializeCheatSystem();

    HotkeyManager::GetSingleton()->RegisterSink();

    SystemMenuInjector::GetSingleton()->Register();

    CheatRegistry::GetSingleton()->RegisterForEvents();

    logger::info("CheatCodeMenu initialized successfully");
    break;

  case SKSE::MessagingInterface::kNewGame:
    logger::info("New Game started");
    CheatRegistry::GetSingleton()->OnNewGame();
    break;

  case SKSE::MessagingInterface::kPostLoadGame:
    logger::info("Game Loaded");
    CheatRegistry::GetSingleton()->OnPostLoadGame();
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

  Serialization::Register();
  SKSE::GetPapyrusInterface()->Register(PapyrusAPI::Register);
  g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

  return true;
}
