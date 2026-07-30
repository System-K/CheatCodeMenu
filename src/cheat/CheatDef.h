#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace CheatCodeMenu {
enum class CheatState : uint8_t {
  Locked = 0,
  Hidden = 1,
  Unlocked = 2
};

enum class CheatMode : uint8_t {
  Trigger,
  Toggle
};

struct CheatAction {
  std::string type;
  nlohmann::json params;
};

struct CheatDef {
  std::string id;
  std::string displayName;
  std::string lockedName;
  std::vector<std::string> codes;
  std::string exclusiveGroup;
  CheatState defaultState;
  bool startEnabled = false;
  CheatMode mode;

  std::vector<CheatAction> actions;
  std::vector<CheatAction> enableActions;
  std::vector<CheatAction> disableActions;
};

struct CheatRuntimeState {
  CheatState state = CheatState::Locked;
  bool toggleEnabled = false;
  bool hasAutoActivated = false;
};

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

inline std::string NormalizeCode(const std::string &code) {
  std::string result = code;

  size_t start = result.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";
  result = result.substr(start);

  size_t end = result.find_last_not_of(" \t\n\r");
  result = result.substr(0, end + 1);

  std::transform(
      result.begin(), result.end(), result.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  return result;
}
} // namespace CheatCodeMenu
