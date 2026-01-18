#include "PapyrusAPI.h"
#include "cheat/CheatExecutor.h"
#include "cheat/CheatRegistry.h"
#include "cheat/CheatStateStore.h"


namespace CheatCodeMenu::PapyrusAPI {
constexpr std::string_view SCRIPT_NAME = "CCM_Native";
constexpr std::string_view ENTER_CODE_KEY = "__enter_code__";

// Get array of menu option labels for ShowList
std::vector<std::string> GetMenuOptionLabels(RE::StaticFunctionTag *) {
  std::vector<std::string> labels;

  // First entry is always "Enter Code..."
  labels.push_back("Enter Code...");

  auto *registry = CheatRegistry::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();

  for (const auto &id : registry->GetAllIds()) {
    const auto *def = registry->GetCheat(id);
    if (!def)
      continue;

    CheatState state = stateStore->GetState(id);

    // Skip hidden cheats
    if (state == CheatState::Hidden)
      continue;

    // Build label
    if (state == CheatState::Unlocked) {
      // Show display name, with toggle indicator if applicable
      if (def->mode == CheatMode::Toggle) {
        bool enabled = stateStore->IsToggleEnabled(id);
        labels.push_back(def->displayName + (enabled ? " [ON]" : " [OFF]"));
      } else {
        labels.push_back(def->displayName);
      }
    } else {
      // Locked - show locked name
      labels.push_back(def->lockedName + " (Locked)");
    }
  }

  return labels;
}

// Get array of menu option keys parallel to labels
std::vector<std::string> GetMenuOptionKeys(RE::StaticFunctionTag *) {
  std::vector<std::string> keys;

  // First entry corresponds to "Enter Code..."
  keys.push_back(std::string(ENTER_CODE_KEY));

  auto *registry = CheatRegistry::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();

  for (const auto &id : registry->GetAllIds()) {
    const auto *def = registry->GetCheat(id);
    if (!def)
      continue;

    CheatState state = stateStore->GetState(id);

    // Skip hidden cheats
    if (state == CheatState::Hidden)
      continue;

    keys.push_back(id);
  }

  return keys;
}

// Submit a code and unlock matching cheats
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

  // Unlock all matching cheats
  for (const auto &id : matchingIds) {
    stateStore->Unlock(id);
  }

  logger::info("Code accepted, unlocked {} cheat(s)", matchingIds.size());
  return true;
}

// Get state of a cheat (0=locked, 1=hidden, 2=unlocked)
int32_t GetCheatState(RE::StaticFunctionTag *, RE::BSFixedString id) {
  auto *stateStore = CheatStateStore::GetSingleton();
  CheatState state = stateStore->GetState(id.c_str());
  return static_cast<int32_t>(state);
}

// Activate a cheat
bool ActivateCheat(RE::StaticFunctionTag *, RE::BSFixedString id) {
  return CheatExecutor::GetSingleton()->ActivateCheat(id.c_str());
}

// Check if toggle cheat is enabled
bool IsCheatEnabled(RE::StaticFunctionTag *, RE::BSFixedString id) {
  auto *registry = CheatRegistry::GetSingleton();
  const auto *def = registry->GetCheat(id.c_str());

  if (!def || def->mode != CheatMode::Toggle) {
    return false;
  }

  return CheatStateStore::GetSingleton()->IsToggleEnabled(id.c_str());
}

// Reload cheat definitions from disk
void ReloadCheatsFromDisk(RE::StaticFunctionTag *) {
  logger::info("Reloading cheat definitions from disk");
  CheatRegistry::GetSingleton()->LoadFromDisk();
  CheatStateStore::GetSingleton()->Initialize();
}

bool Register(RE::BSScript::IVirtualMachine *vm) {
  vm->RegisterFunction("GetMenuOptionLabels", SCRIPT_NAME, GetMenuOptionLabels);
  vm->RegisterFunction("GetMenuOptionKeys", SCRIPT_NAME, GetMenuOptionKeys);
  vm->RegisterFunction("SubmitCode", SCRIPT_NAME, SubmitCode);
  vm->RegisterFunction("GetCheatState", SCRIPT_NAME, GetCheatState);
  vm->RegisterFunction("ActivateCheat", SCRIPT_NAME, ActivateCheat);
  vm->RegisterFunction("IsCheatEnabled", SCRIPT_NAME, IsCheatEnabled);
  vm->RegisterFunction("ReloadCheatsFromDisk", SCRIPT_NAME,
                       ReloadCheatsFromDisk);

  logger::info("Registered {} Papyrus native functions", 7);
  return true;
}
} // namespace CheatCodeMenu::PapyrusAPI
