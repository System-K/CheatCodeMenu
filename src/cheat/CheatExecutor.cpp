#include "CheatExecutor.h"
#include "CheatRegistry.h"
#include "CheatStateStore.h"

namespace CheatCodeMenu {
CheatExecutor *CheatExecutor::GetSingleton() {
  static CheatExecutor singleton;
  return &singleton;
}

size_t CheatExecutor::ExecuteActions(const std::vector<CheatAction> &actions) {
  size_t executedCount = 0;

  for (const auto &action : actions) {
    if (action.type == "console") {
      if (action.params.contains("command") &&
          action.params["command"].is_string()) {
        if (ExecuteConsoleCommand(
                action.params["command"].get<std::string>())) {
          executedCount++;
        }
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

        if (SendModEvent(eventName, strArg, numArg)) {
          executedCount++;
        }
      } else {
        logger::warn("ModEvent action missing 'eventName' parameter");
      }
    } else {
      logger::warn("Unknown action type: {}", action.type);
    }
  }

  return executedCount;
}

size_t CheatExecutor::ExecuteToggleActions(const CheatDef &def, bool enabling) {
  size_t executedCount = ExecuteActions(def.actions);
  executedCount +=
      ExecuteActions(enabling ? def.enableActions : def.disableActions);
  return executedCount;
}

bool CheatExecutor::ActivateCheat(const std::string &id, bool force) {
  auto *registry = CheatRegistry::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();

  const auto *def = registry->GetCheat(id);
  if (!def) {
    logger::error("ActivateCheat: Unknown cheat ID '{}'", id);
    return false;
  }

  CheatState state = stateStore->GetState(id);
  if (!force && state != CheatState::Unlocked) {
    logger::info("ActivateCheat: Cheat '{}' is not unlocked", id);
    return false;
  }

  logger::info("Activating cheat '{}'", id);

  if (def->mode == CheatMode::Trigger) {
    if (ExecuteActions(def->actions) == 0) {
      logger::warn(
          "ActivateCheat: Cheat '{}' has no valid trigger actions to execute",
          id);
      return false;
    }
  } else {
    bool currentlyEnabled = stateStore->IsToggleEnabled(id);
    bool newState = !currentlyEnabled;

    if (newState && !def->exclusiveGroup.empty()) {
      auto groupCheats = registry->GetCheatsInGroup(def->exclusiveGroup);
      for (const auto &otherId : groupCheats) {
        if (otherId == id)
          continue;

        if (stateStore->IsToggleEnabled(otherId)) {
          const auto *otherDef = registry->GetCheat(otherId);
          if (otherDef) {
            logger::info(
                "Exclusive group '{}': disabling '{}' before enabling '{}'",
                def->exclusiveGroup, otherId, id);
            if (ExecuteToggleActions(*otherDef, false) == 0) {
              logger::warn(
                  "ActivateCheat: Failed to disable '{}' while enabling '{}'",
                  otherId, id);
              return false;
            }
            stateStore->SetToggleEnabled(otherId, false);
          }
        }
      }
    }

    if (ExecuteToggleActions(*def, newState) == 0) {
      logger::warn(
          "ActivateCheat: Cheat '{}' has no valid {} actions to execute", id,
          newState ? "enable" : "disable");
      return false;
    }

    if (newState) {
      stateStore->SetToggleEnabled(id, true);
      logger::info("Cheat '{}' enabled", id);
    } else {
      stateStore->SetToggleEnabled(id, false);
      logger::info("Cheat '{}' disabled", id);
    }
  }

  return true;
}

bool CheatExecutor::ExecuteConsoleCommand(const std::string &command) {
  if (command.empty()) {
    logger::warn("Empty console command, skipping");
    return false;
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

  return true;
}

bool CheatExecutor::SendModEvent(const std::string &eventName,
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
    return true;
  } else {
    logger::error("Failed to get ModCallbackEventSource");
    return false;
  }
}
} // namespace CheatCodeMenu
