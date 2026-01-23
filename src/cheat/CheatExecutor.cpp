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
    } else if (action.type == "modevent") {
      if (action.params.contains("eventName") &&
          action.params["eventName"].is_string()) {
        std::string eventName = action.params["eventName"].get<std::string>();
        std::string strArg = "";
        float numArg = 0.0f;

        if (action.params.contains("strArg") &&
            action.params["strArg"].is_string()) {
          strArg = action.params["strArg"].get<std::string>();
        }
        if (action.params.contains("numArg") &&
            action.params["numArg"].is_number()) {
          numArg = action.params["numArg"].get<float>();
        }

        SendModEvent(eventName, strArg, numArg);
      } else {
        logger::warn("ModEvent action missing 'eventName' parameter");
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

  // Queue on main thread to ensure safe execution
  std::string cmdCopy = command;
  SKSE::GetTaskInterface()->AddTask([cmdCopy]() {
    const auto scriptFactory =
        RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
    if (!scriptFactory) {
      logger::error("Failed to get Script factory");
      return;
    }

    const auto script = scriptFactory->Create();
    if (!script) {
      logger::error("Failed to create Script");
      return;
    }

    script->SetCommand(cmdCopy);
    script->CompileAndRun(nullptr);
    delete script;
  });
}

void CheatExecutor::SendModEvent(const std::string &eventName,
                                 const std::string &strArg, float numArg) {
  logger::info("Sending mod event: {} (strArg='{}', numArg={})", eventName,
               strArg, numArg);

  auto *eventSource = SKSE::GetModCallbackEventSource();
  if (eventSource) {
    SKSE::ModCallbackEvent event;
    event.eventName = eventName;
    event.strArg = strArg;
    event.numArg = numArg;
    event.sender = nullptr;
    eventSource->SendEvent(&event);
  } else {
    logger::error("Failed to get ModCallbackEventSource");
  }
}
} // namespace CheatCodeMenu
