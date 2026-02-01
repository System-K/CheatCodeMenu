#pragma once

#include "CheatDef.h"
#include <string>
#include <vector>

namespace CheatCodeMenu {
class CheatExecutor {
public:
  static CheatExecutor *GetSingleton();

  // Execute a list of actions
  void ExecuteActions(const std::vector<CheatAction> &actions);

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

  void ExecuteConsoleCommand(const std::string &command);
  void SendModEvent(const std::string &eventName, const std::string &strArg,
                    float numArg);
};
} // namespace CheatCodeMenu
