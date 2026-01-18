#pragma once

#include "CheatDef.h"
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>


namespace CheatCodeMenu {
class CheatRegistry {
public:
  static CheatRegistry *GetSingleton();

  // Load all cheat definitions from disk
  void LoadFromDisk();

  // Get a cheat definition by ID (nullptr if not found)
  const CheatDef *GetCheat(const std::string &id) const;

  // Look up which cheat IDs match a given code (normalized, case-insensitive)
  std::vector<std::string> LookupCode(const std::string &code) const;

  // Get all registered cheat IDs in registration order
  const std::vector<std::string> &GetAllIds() const { return m_orderedIds; }

  // Get total count of loaded cheats
  size_t GetCount() const { return m_cheats.size(); }

  // Clear all loaded cheats (for reload)
  void Clear();

private:
  CheatRegistry() = default;
  ~CheatRegistry() = default;

  CheatRegistry(const CheatRegistry &) = delete;
  CheatRegistry(CheatRegistry &&) = delete;
  CheatRegistry &operator=(const CheatRegistry &) = delete;
  CheatRegistry &operator=(CheatRegistry &&) = delete;

  // Load a single JSON file
  bool LoadFile(const std::filesystem::path &filepath);

  // Parse a single cheat from JSON
  bool ParseCheat(const nlohmann::json &j, CheatDef &outCheat,
                  const std::string &filename);

  // Parse action array
  std::vector<CheatAction> ParseActions(const nlohmann::json &j);

  // Cheat definitions by ID
  std::unordered_map<std::string, CheatDef> m_cheats;

  // Ordered list of IDs (for consistent menu ordering)
  std::vector<std::string> m_orderedIds;

  // Code to cheat ID mapping (normalized codes)
  std::unordered_multimap<std::string, std::string> m_codeToIds;
};
} // namespace CheatCodeMenu
