#include "PapyrusAPI.h"
#include "ConfigManager.h"
#include "cheat/CheatExecutor.h"
#include "cheat/CheatRegistry.h"
#include "cheat/CheatStateStore.h"

namespace CheatCodeMenu::PapyrusAPI {
constexpr std::string_view SCRIPT_NAME = "CCM_Native";
constexpr std::string_view ENTER_CODE_KEY = "__enter_code__";

std::vector<std::string> GetMenuOptionLabels(RE::StaticFunctionTag *) {
  std::vector<std::string> labels;

  auto *registry = CheatRegistry::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();

  if (registry->GetAllIds().empty()) {
    labels.push_back("No Cheats Installed");
    return labels;
  }

  bool hasNonUnlockedCheats = false;
  for (const auto &id : registry->GetAllIds()) {
    const auto *def = registry->GetCheat(id);
    if (!def)
      continue;

    CheatState state = stateStore->GetState(id);
    if (state != CheatState::Unlocked) {
      hasNonUnlockedCheats = true;
      break;
    }
  }

  if (hasNonUnlockedCheats) {
    labels.push_back("Enter Code...");
  }

  for (const auto &id : registry->GetAllIds()) {
    const auto *def = registry->GetCheat(id);
    if (!def)
      continue;

    CheatState state = stateStore->GetState(id);

    if (state == CheatState::Hidden)
      continue;

    if (state == CheatState::Unlocked) {
      if (def->mode == CheatMode::Toggle) {
        bool enabled = stateStore->IsToggleEnabled(id);
        labels.push_back(def->displayName + (enabled ? " [ON]" : " [OFF]"));
      } else {
        labels.push_back(def->displayName);
      }
    } else {
      labels.push_back(def->lockedName + " (Locked)");
    }
  }

  return labels;
}

std::vector<std::string> GetMenuOptionKeys(RE::StaticFunctionTag *) {
  std::vector<std::string> keys;

  auto *registry = CheatRegistry::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();

  if (registry->GetAllIds().empty()) {
    keys.push_back("");
    return keys;
  }

  bool hasNonUnlockedCheats = false;
  for (const auto &id : registry->GetAllIds()) {
    const auto *def = registry->GetCheat(id);
    if (!def)
      continue;

    CheatState state = stateStore->GetState(id);
    if (state != CheatState::Unlocked) {
      hasNonUnlockedCheats = true;
      break;
    }
  }

  if (hasNonUnlockedCheats) {
    keys.push_back(std::string(ENTER_CODE_KEY));
  }

  for (const auto &id : registry->GetAllIds()) {
    const auto *def = registry->GetCheat(id);
    if (!def)
      continue;

    CheatState state = stateStore->GetState(id);

    if (state == CheatState::Hidden)
      continue;

    keys.push_back(id);
  }

  return keys;
}

bool SubmitCode(RE::StaticFunctionTag *, RE::BSFixedString code) {
  if (code.empty()) {
    return false;
  }

  auto *registry = CheatRegistry::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();

  std::string codeStr = code.c_str();
  auto matchingIds = registry->LookupCode(codeStr);

  if (matchingIds.empty()) {
    logger::info("Code submitted but no matches found");
    return false;
  }

  for (const auto &id : matchingIds) {
    stateStore->Unlock(id);
  }

  logger::info("Code accepted, unlocked {} cheat(s)", matchingIds.size());
  return true;
}

int32_t GetCheatState(RE::StaticFunctionTag *, RE::BSFixedString id) {
  auto *stateStore = CheatStateStore::GetSingleton();
  CheatState state = stateStore->GetState(id.c_str());
  return static_cast<int32_t>(state);
}

bool ActivateCheat(RE::StaticFunctionTag *, RE::BSFixedString id) {
  return CheatExecutor::GetSingleton()->ActivateCheat(id.c_str());
}

bool IsCheatEnabled(RE::StaticFunctionTag *, RE::BSFixedString id) {
  auto *registry = CheatRegistry::GetSingleton();
  const auto *def = registry->GetCheat(id.c_str());

  if (!def || def->mode != CheatMode::Toggle) {
    return false;
  }

  return CheatStateStore::GetSingleton()->IsToggleEnabled(id.c_str());
}

void ReloadCheatsFromDisk(RE::StaticFunctionTag *) {
  logger::info("Reloading cheat definitions from disk");
  CheatRegistry::GetSingleton()->LoadFromDisk();
  CheatStateStore::GetSingleton()->Initialize();
}

bool IsTriggerMode(RE::StaticFunctionTag *, RE::BSFixedString id) {
  auto *registry = CheatRegistry::GetSingleton();
  const auto *def = registry->GetCheat(id.c_str());
  if (!def) {
    return false;
  }
  return def->mode == CheatMode::Trigger;
}

RE::BSFixedString GetMessage(RE::StaticFunctionTag *, RE::BSFixedString key) {
  return ConfigManager::GetSingleton()->GetMessage(key.c_str());
}

bool Register(RE::BSScript::IVirtualMachine *vm) {
  vm->RegisterFunction("GetMenuOptionLabels", SCRIPT_NAME, GetMenuOptionLabels);
  vm->RegisterFunction("GetMenuOptionKeys", SCRIPT_NAME, GetMenuOptionKeys);
  vm->RegisterFunction("SubmitCode", SCRIPT_NAME, SubmitCode);
  vm->RegisterFunction("GetCheatState", SCRIPT_NAME, GetCheatState);
  vm->RegisterFunction("ActivateCheat", SCRIPT_NAME, ActivateCheat);
  vm->RegisterFunction("IsCheatEnabled", SCRIPT_NAME, IsCheatEnabled);
  vm->RegisterFunction("IsTriggerMode", SCRIPT_NAME, IsTriggerMode);
  vm->RegisterFunction("ReloadCheatsFromDisk", SCRIPT_NAME,
                       ReloadCheatsFromDisk);
  vm->RegisterFunction("GetMessage", SCRIPT_NAME, GetMessage);

  logger::info("Registered {} Papyrus native functions", 9);
  return true;
}
} // namespace CheatCodeMenu::PapyrusAPI
