#include "CheatExecutor.h"
#include "CheatRegistry.h"
#include "CheatStateStore.h"

namespace CheatCodeMenu {
CheatExecutor *CheatExecutor::GetSingleton() {
  static CheatExecutor singleton;
  return &singleton;
}

void CheatExecutor::ExecuteActions(const std::vector<CheatAction> &actions) {
  for (const auto &action : actions) {
    if (action.type == "console") {
      if (action.params.contains("command") &&
          action.params["command"].is_string()) {
        ExecuteConsoleCommand(action.params["command"].get<std::string>());
      } else {
        logger::warn("Console action missing 'command' parameter");
      }
    } else {
      logger::warn("Unknown action type: {}", action.type);
    }
  }
}

bool CheatExecutor::ActivateCheat(const std::string &id) {
  auto *registry = CheatRegistry::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();

  const auto *def = registry->GetCheat(id);
  if (!def) {
    logger::error("ActivateCheat: Unknown cheat ID '{}'", id);
    return false;
  }

  CheatState state = stateStore->GetState(id);
  if (state != CheatState::Unlocked) {
    logger::info("ActivateCheat: Cheat '{}' is not unlocked", id);
    return false;
  }

  logger::info("Activating cheat '{}'", id);

  if (def->mode == CheatMode::Trigger) {
    // Trigger mode: execute actions once
    ExecuteActions(def->actions);
  } else {
    // Toggle mode: flip state and execute corresponding actions
    bool currentlyEnabled = stateStore->IsToggleEnabled(id);
    bool newState = !currentlyEnabled;
    stateStore->SetToggleEnabled(id, newState);

    if (newState) {
      ExecuteActions(def->enableActions);
      logger::info("Cheat '{}' enabled", id);
    } else {
      ExecuteActions(def->disableActions);
      logger::info("Cheat '{}' disabled", id);
    }
  }

  return true;
}

void CheatExecutor::ExecuteConsoleCommand(const std::string &command) {
  if (command.empty()) {
    logger::warn("Empty console command, skipping");
    return;
  }

  logger::info("Executing console command: {}", command);

  // Execute via Script::Execute
  const auto scriptFactory =
      RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
  if (scriptFactory) {
    const auto script = scriptFactory->Create();
    if (script) {
      script->SetCommand(command);
      script->CompileAndRun(nullptr);
      delete script;
    }
  }
}
} // namespace CheatCodeMenu
