#pragma once

#include <string>
#include <unordered_map>

namespace CheatCodeMenu {
class ConfigManager {
public:
  static ConfigManager *GetSingleton();

  void LoadConfig();
  std::string GetMessage(const std::string &key) const;
  bool GetUnlockAllCodes() const;

private:
  ConfigManager() = default;
  ConfigManager(const ConfigManager &) = delete;
  ConfigManager &operator=(const ConfigManager &) = delete;

  std::unordered_map<std::string, std::string> _messages;
  bool _unlockAllCodes = false;

  void LoadMessages(const std::string &path);
};
} // namespace CheatCodeMenu
