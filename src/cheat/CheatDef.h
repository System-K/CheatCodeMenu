#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace CheatCodeMenu {
// Cheat visibility/activation state
enum class CheatState : uint8_t {
  Locked = 0,  // Visible but not activatable
  Hidden = 1,  // Not visible until unlocked
  Unlocked = 2 // Visible and activatable
};

// Cheat activation mode
enum class CheatMode : uint8_t {
  Trigger, // Execute actions once per activation
  Toggle   // Flip enabled state, execute enable/disable actions
};

// Single action to execute
struct CheatAction {
  std::string type;      // "console", future: "event", "spell", etc.
  nlohmann::json params; // Type-specific parameters
};

// Cheat definition loaded from JSON
struct CheatDef {
  std::string id;          // Unique identifier
  std::string displayName; // Shown when unlocked
  std::string lockedName;  // Shown when locked (default: "?????")
  std::vector<std::string>
      codes; // Code phrases to unlock (normalized for comparison)
  std::string
      exclusiveGroup; // Optional: only one cheat per group can be active
  CheatState defaultState;
  bool startEnabled = false;
  CheatMode mode;

  // Actions for trigger mode, or general actions for toggle
  std::vector<CheatAction> actions;

  // Toggle mode only
  std::vector<CheatAction> enableActions;
  std::vector<CheatAction> disableActions;
};

// Runtime state for a cheat (persisted in co-save)
struct CheatRuntimeState {
  CheatState state = CheatState::Locked;
  bool toggleEnabled = false;
  bool hasAutoActivated = false;
};

// Helper functions
inline std::optional<CheatState> StateFromString(const std::string &str) {
  if (str == "hidden")
    return CheatState::Hidden;
  if (str == "unlocked")
    return CheatState::Unlocked;
  if (str == "locked")
    return CheatState::Locked;
  return std::nullopt;
}

inline std::optional<CheatMode> ModeFromString(const std::string &str) {
  if (str == "toggle")
    return CheatMode::Toggle;
  if (str == "trigger")
    return CheatMode::Trigger;
  return std::nullopt;
}

inline std::string StateToString(CheatState state) {
  switch (state) {
  case CheatState::Hidden:
    return "hidden";
  case CheatState::Unlocked:
    return "unlocked";
  default:
    return "locked";
  }
}

inline std::string ModeToString(CheatMode mode) {
  switch (mode) {
  case CheatMode::Toggle:
    return "toggle";
  default:
    return "trigger";
  }
}

// Normalize code for comparison (trim whitespace, lowercase)
inline std::string NormalizeCode(const std::string &code) {
  std::string result = code;

  // Trim leading whitespace
  size_t start = result.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";
  result = result.substr(start);

  // Trim trailing whitespace
  size_t end = result.find_last_not_of(" \t\n\r");
  result = result.substr(0, end + 1);

  // Convert to lowercase
  std::transform(
      result.begin(), result.end(), result.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  return result;
}
} // namespace CheatCodeMenu
