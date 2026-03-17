#include "CheatRegistry.h"
#include "CheatExecutor.h"
#include "CheatStateStore.h"
#include <algorithm>
#include <fstream>
#include <unordered_set>

namespace CheatCodeMenu {
CheatRegistry *CheatRegistry::GetSingleton() {
  static CheatRegistry singleton;
  return &singleton;
}

void CheatRegistry::LoadFromDisk() {
  Clear();

  // Build path to cheats directory
  auto dataPath =
      std::filesystem::path("Data/SKSE/Plugins/CheatCodeMenu/cheats");

  if (!std::filesystem::exists(dataPath)) {
    logger::info("Cheats directory does not exist: {}", dataPath.string());
    logger::info("Running with zero cheat definitions");
    return;
  }

  if (!std::filesystem::is_directory(dataPath)) {
    logger::warn("Cheats path is not a directory: {}", dataPath.string());
    return;
  }

  size_t filesLoaded = 0;
  size_t filesFailed = 0;

  for (const auto &entry : std::filesystem::directory_iterator(dataPath)) {
    if (!entry.is_regular_file())
      continue;

    auto ext = entry.path().extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });

    if (ext != ".json")
      continue;

    if (LoadFile(entry.path())) {
      filesLoaded++;
    } else {
      filesFailed++;
    }
  }

  logger::info("Loaded {} cheat definition files ({} failed), {} total cheats",
               filesLoaded, filesFailed, m_cheats.size());

  ApplySortOrder();
}

void CheatRegistry::ApplySortOrder() {
  if (m_explicitSortOrder.empty()) {
    return;
  }

  std::vector<std::string> newOrder;
  std::unordered_set<std::string> addedIds;

  // First add explicitly ordered IDs if they exist
  for (const auto &id : m_explicitSortOrder) {
    if (m_cheats.contains(id)) {
      newOrder.push_back(id);
      addedIds.insert(id);
    }
  }

  // Then add remaining IDs in their original load order
  for (const auto &id : m_orderedIds) {
    if (addedIds.find(id) == addedIds.end()) {
      newOrder.push_back(id);
    }
  }

  m_orderedIds = std::move(newOrder);
  logger::info("Applied explicit sort order, {} cheats total",
               m_orderedIds.size());
}

void CheatRegistry::RegisterForEvents() {
  auto *ui = RE::UI::GetSingleton();
  if (ui) {
    ui->AddEventSink(this);
    logger::info("Registered for MenuOpenCloseEvent");
  }
}

void CheatRegistry::OnNewGame() {
  m_pendingNewGameCheats = true;
  logger::info("New Game detected, waiting for RaceMenu exit to apply cheats");
}

void CheatRegistry::OnPostLoadGame() {
  // Logic for post-load (e.g., session resume)
  // For startEnabled, we check if they were already activated.
  // If not, and they are set to startEnabled, we activate them.
  ApplyDefaultEnabledState();
}

RE::BSEventNotifyControl
CheatRegistry::ProcessEvent(const RE::MenuOpenCloseEvent *a_event,
                            RE::BSTEventSource<RE::MenuOpenCloseEvent> *) {
  if (!a_event || m_pendingNewGameCheats == false) {
    return RE::BSEventNotifyControl::kContinue;
  }

  // "RaceSex Menu" is the internal name for the character creation menu
  if (a_event->menuName == "RaceSex Menu" && !a_event->opening) {
    logger::info("RaceMenu closed on New Game - applying default cheats");
    ApplyDefaultEnabledState();
    m_pendingNewGameCheats = false;
  }

  return RE::BSEventNotifyControl::kContinue;
}

void CheatRegistry::ApplyDefaultEnabledState() {
  auto *executor = CheatExecutor::GetSingleton();
  auto *stateStore = CheatStateStore::GetSingleton();
  int count = 0;

  for (const auto &[id, def] : m_cheats) {
    if (def.startEnabled) {
      if (stateStore->HasAutoActivated(id)) {
        continue; // Already ran for this save
      }

      // Check toggle state for logic - if it's toggle mode and already enabled
      // (manually?), skip? But HasAutoActivated handles the "only run once"
      // rule. So we just activate it now.

      bool shouldActivate = true;
      if (def.mode == CheatMode::Toggle) {
        if (stateStore->IsToggleEnabled(id)) {
          shouldActivate = false; // Already on
        }
      }

      if (shouldActivate) {
        if (executor->ActivateCheat(id, true)) {
          stateStore->SetHasAutoActivated(id, true);
          count++;
        } else {
          logger::warn("Failed to auto-activate cheat '{}'", id);
        }
      } else {
        stateStore->SetHasAutoActivated(id, true);
      }
    }
  }

  if (count > 0) {
    logger::info("Auto-activated {} cheats", count);
  }
}

bool CheatRegistry::LoadFile(const std::filesystem::path &filepath) {
  logger::info("Loading cheat file: {}", filepath.filename().string());

  std::ifstream file(filepath);
  if (!file.is_open()) {
    logger::error("Failed to open file: {}", filepath.string());
    return false;
  }

  nlohmann::json root;
  try {
    file >> root;
  } catch (const nlohmann::json::parse_error &e) {
    logger::error("JSON parse error in {}: {}", filepath.filename().string(),
                  e.what());
    return false;
  }

  // Validate schema version
  if (!root.contains("schemaVersion") ||
      !root["schemaVersion"].is_number_integer()) {
    logger::error("Missing or invalid schemaVersion in {}",
                  filepath.filename().string());
    return false;
  }

  int schemaVersion = root["schemaVersion"].get<int>();
  if (schemaVersion != 1) {
    logger::error("Unsupported schemaVersion {} in {} (expected 1)",
                  schemaVersion, filepath.filename().string());
    return false;
  }

  // Parse sortOrder (optional top-level)
  if (root.contains("sortOrder") && root["sortOrder"].is_array()) {
    for (const auto &idJson : root["sortOrder"]) {
      if (idJson.is_string()) {
        m_explicitSortOrder.push_back(idJson.get<std::string>());
      }
    }
    logger::info("  Loaded {} explicitly ordered IDs from {}",
                 root["sortOrder"].size(), filepath.filename().string());
  }

  // Parse cheats array
  if (!root.contains("cheats") || !root["cheats"].is_array()) {
    logger::error("Missing or invalid 'cheats' array in {}",
                  filepath.filename().string());
    return false;
  }

  size_t cheatsLoaded = 0;
  for (const auto &cheatJson : root["cheats"]) {
    CheatDef cheat;
    if (ParseCheat(cheatJson, cheat, filepath.filename().string())) {
      // Check for duplicate ID
      if (m_cheats.contains(cheat.id)) {
        logger::warn("Duplicate cheat ID '{}' in {}, skipping", cheat.id,
                     filepath.filename().string());
        continue;
      }

      // Register all codes for this cheat
      for (const auto &code : cheat.codes) {
        std::string normalizedCode = NormalizeCode(code);
        m_codeToIds.emplace(normalizedCode, cheat.id);
      }

      // Register exclusive group if present
      if (!cheat.exclusiveGroup.empty()) {
        m_groupToIds.emplace(cheat.exclusiveGroup, cheat.id);
      }

      m_orderedIds.push_back(cheat.id);
      m_cheats[cheat.id] = std::move(cheat);
      cheatsLoaded++;
    }
  }

  logger::info("  Loaded {} cheats from {}", cheatsLoaded,
               filepath.filename().string());
  return true;
}

bool CheatRegistry::ParseCheat(const nlohmann::json &j, CheatDef &outCheat,
                               const std::string &filename) {
  // Required fields
  if (!j.contains("id") || !j["id"].is_string()) {
    logger::error("Cheat missing 'id' in {}", filename);
    return false;
  }
  outCheat.id = j["id"].get<std::string>();

  if (!j.contains("displayName") || !j["displayName"].is_string()) {
    logger::error("Cheat '{}' missing 'displayName' in {}", outCheat.id,
                  filename);
    return false;
  }
  outCheat.displayName = j["displayName"].get<std::string>();

  // Parse codes - support both "code" (single) and "codes" (array)
  if (j.contains("codes") && j["codes"].is_array()) {
    for (const auto &codeJson : j["codes"]) {
      if (codeJson.is_string()) {
        outCheat.codes.push_back(codeJson.get<std::string>());
      }
    }
  } else if (j.contains("code") && j["code"].is_string()) {
    outCheat.codes.push_back(j["code"].get<std::string>());
  }

  if (outCheat.codes.empty()) {
    logger::error("Cheat '{}' missing 'code' or 'codes' in {}", outCheat.id,
                  filename);
    return false;
  }

  if (!j.contains("defaultState") || !j["defaultState"].is_string()) {
    logger::error("Cheat '{}' missing 'defaultState' in {}", outCheat.id,
                  filename);
    return false;
  }
  const auto defaultState =
      StateFromString(j["defaultState"].get<std::string>());
  if (!defaultState.has_value()) {
    logger::error("Cheat '{}' has invalid defaultState '{}' in {}",
                  outCheat.id, j["defaultState"].get<std::string>(), filename);
    return false;
  }
  outCheat.defaultState = *defaultState;

  if (!j.contains("mode") || !j["mode"].is_string()) {
    logger::error("Cheat '{}' missing 'mode' in {}", outCheat.id, filename);
    return false;
  }
  const auto mode = ModeFromString(j["mode"].get<std::string>());
  if (!mode.has_value()) {
    logger::error("Cheat '{}' has invalid mode '{}' in {}", outCheat.id,
                  j["mode"].get<std::string>(), filename);
    return false;
  }
  outCheat.mode = *mode;

  // Optional fields
  if (j.contains("lockedName") && j["lockedName"].is_string()) {
    outCheat.lockedName = j["lockedName"].get<std::string>();
  } else {
    outCheat.lockedName = "?????";
  }

  // Exclusive group (optional)
  if (j.contains("exclusiveGroup") && j["exclusiveGroup"].is_string()) {
    outCheat.exclusiveGroup = j["exclusiveGroup"].get<std::string>();
  }

  // Start enabled (optional)
  if (j.contains("startEnabled") && j["startEnabled"].is_boolean()) {
    outCheat.startEnabled = j["startEnabled"].get<bool>();
  }

  // Actions
  if (j.contains("actions")) {
    if (!j["actions"].is_array()) {
      logger::error("Cheat '{}' has non-array 'actions' field in {}",
                    outCheat.id, filename);
      return false;
    }
    if (!ParseActions(j["actions"], outCheat.actions, filename, outCheat.id,
                      "actions")) {
      return false;
    }
  }

  if (j.contains("enableActions")) {
    if (!j["enableActions"].is_array()) {
      logger::error("Cheat '{}' has non-array 'enableActions' field in {}",
                    outCheat.id, filename);
      return false;
    }
    if (!ParseActions(j["enableActions"], outCheat.enableActions, filename,
                      outCheat.id, "enableActions")) {
      return false;
    }
  }

  if (j.contains("disableActions")) {
    if (!j["disableActions"].is_array()) {
      logger::error("Cheat '{}' has non-array 'disableActions' field in {}",
                    outCheat.id, filename);
      return false;
    }
    if (!ParseActions(j["disableActions"], outCheat.disableActions, filename,
                      outCheat.id, "disableActions")) {
      return false;
    }
  }

  return true;
}

bool CheatRegistry::ParseActions(const nlohmann::json &j,
                                 std::vector<CheatAction> &outActions,
                                 const std::string &filename,
                                 const std::string &cheatId,
                                 const char *fieldName) {
  outActions.clear();

  for (size_t index = 0; index < j.size(); ++index) {
    const auto &actionJson = j[index];
    if (!actionJson.is_object()) {
      logger::error("Cheat '{}' has non-object {}[{}] in {}", cheatId,
                    fieldName, index, filename);
      return false;
    }
    if (!actionJson.contains("type") || !actionJson["type"].is_string()) {
      logger::error("Cheat '{}' has {}[{}] missing string 'type' in {}",
                    cheatId, fieldName, index, filename);
      return false;
    }

    CheatAction action;
    action.type = actionJson["type"].get<std::string>();

    if (!actionJson.contains("params") || !actionJson["params"].is_object()) {
      logger::error("Cheat '{}' has {}[{}] missing object 'params' in {}",
                    cheatId, fieldName, index, filename);
      return false;
    }
    action.params = actionJson["params"];

    if (action.type == "console") {
      if (!action.params.contains("command") ||
          !action.params["command"].is_string() ||
          action.params["command"].get<std::string>().empty()) {
        logger::error(
            "Cheat '{}' has {}[{}] with invalid console 'command' in {}",
            cheatId, fieldName, index, filename);
        return false;
      }
    } else if (action.type == "modevent") {
      if (!action.params.contains("eventName") ||
          !action.params["eventName"].is_string() ||
          action.params["eventName"].get<std::string>().empty()) {
        logger::error(
            "Cheat '{}' has {}[{}] with invalid modevent 'eventName' in {}",
            cheatId, fieldName, index, filename);
        return false;
      }
      if (action.params.contains("strArg") &&
          !action.params["strArg"].is_string()) {
        logger::error(
            "Cheat '{}' has {}[{}] with non-string modevent 'strArg' in {}",
            cheatId, fieldName, index, filename);
        return false;
      }
      if (action.params.contains("numArg") &&
          !action.params["numArg"].is_number()) {
        logger::error(
            "Cheat '{}' has {}[{}] with non-numeric modevent 'numArg' in {}",
            cheatId, fieldName, index, filename);
        return false;
      }
    } else {
      logger::error("Cheat '{}' has {}[{}] with unsupported type '{}' in {}",
                    cheatId, fieldName, index, action.type, filename);
      return false;
    }

    outActions.push_back(std::move(action));
  }

  return true;
}

const CheatDef *CheatRegistry::GetCheat(const std::string &id) const {
  auto it = m_cheats.find(id);
  if (it != m_cheats.end()) {
    return &it->second;
  }
  return nullptr;
}

std::vector<std::string>
CheatRegistry::LookupCode(const std::string &code) const {
  std::vector<std::string> result;
  std::string normalizedCode = NormalizeCode(code);

  auto range = m_codeToIds.equal_range(normalizedCode);
  for (auto it = range.first; it != range.second; ++it) {
    result.push_back(it->second);
  }

  return result;
}

std::vector<std::string>
CheatRegistry::GetCheatsInGroup(const std::string &group) const {
  std::vector<std::string> result;
  if (group.empty()) {
    return result;
  }

  auto range = m_groupToIds.equal_range(group);
  for (auto it = range.first; it != range.second; ++it) {
    result.push_back(it->second);
  }

  return result;
}

void CheatRegistry::Clear() {
  m_cheats.clear();
  m_orderedIds.clear();
  m_explicitSortOrder.clear();
  m_codeToIds.clear();
  m_groupToIds.clear();
  m_pendingNewGameCheats = false;
}
} // namespace CheatCodeMenu
