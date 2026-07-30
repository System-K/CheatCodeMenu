#include "ConfigManager.h"
#include <CLIBUtil/detail/SimpleIni.h>
#include <filesystem>

namespace CheatCodeMenu {
ConfigManager *ConfigManager::GetSingleton() {
  static ConfigManager singleton;
  return &singleton;
}

void ConfigManager::LoadConfig() {
  auto path = std::filesystem::path("Data/SKSE/Plugins/CheatCodeMenu.ini");
  LoadMessages(path.string());
}

void ConfigManager::LoadMessages(const std::string &path) {
  CSimpleIniA ini;
  ini.SetUnicode();

  if (ini.LoadFile(path.c_str()) != SI_OK) {
    logger::info("No INI file found at {}, using default messages", path);
    return;
  }

  _unlockAllCodes = ini.GetBoolValue("Settings", "UnlockAllCodes", false);
  logger::info("UnlockAllCodes setting: {}", _unlockAllCodes);

  std::vector<std::pair<std::string, std::string>> defaults = {
      {"CodeAccepted", "Code accepted!"},
      {"InvalidCode", "Invalid code."},
      {"CheatExecuted", "Cheat executed!"},
      {"CheatEnabled", "Cheat enabled!"},
      {"CheatDisabled", "Cheat disabled."},
      {"ActivationFailed", "Activation failed."},
      {"CheatLocked", "This cheat is locked."},
  };

  for (const auto &[key, defaultVal] : defaults) {
    const char *val = ini.GetValue("Messages", key.c_str(), defaultVal.c_str());
    _messages[key] = val;
    logger::info("Loaded message '{}': '{}'", key, val);
  }
}

std::string ConfigManager::GetMessage(const std::string &key) const {
  auto it = _messages.find(key);
  if (it != _messages.end()) {
    return it->second;
  }

  if (key == "CodeAccepted")
    return "Code accepted!";
  if (key == "InvalidCode")
    return "Invalid code.";
  if (key == "CheatExecuted")
    return "Cheat executed!";
  if (key == "CheatEnabled")
    return "Cheat enabled!";
  if (key == "CheatDisabled")
    return "Cheat disabled.";
  if (key == "ActivationFailed")
    return "Activation failed.";
  if (key == "CheatLocked")
    return "This cheat is locked.";

  return key;
}

bool ConfigManager::GetUnlockAllCodes() const {
  return _unlockAllCodes;
}
} // namespace CheatCodeMenu
