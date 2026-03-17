#pragma once

#include "CheatDef.h"
#include <cstddef>
#include <string>
#include <vector>

namespace CheatCodeMenu {
class CheatExecutor {
public:
  static CheatExecutor *GetSingleton();

  // Execute a list of actions and return how many were accepted for execution.
  size_t ExecuteActions(const std::vector<CheatAction> &actions);

  // Activate a cheat by ID
  // Returns true if activation succeeded
  bool ActivateCheat(const std::string &id, bool force = false);

private:
  CheatExecutor() = default;
  ~CheatExecutor() = default;

  CheatExecutor(const CheatExecutor &) = delete;
  CheatExecutor(CheatExecutor &&) = delete;
  CheatExecutor &operator=(const CheatExecutor &) = delete;
  CheatExecutor &operator=(CheatExecutor &&) = delete;

  bool ExecuteConsoleCommand(const std::string &command);
  size_t ExecuteToggleActions(const CheatDef &def, bool enabling);
  bool SendModEvent(const std::string &eventName, const std::string &strArg,
                    float numArg);
};
} // namespace CheatCodeMenu
