#pragma once

#include "CheatDef.h"
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace CheatCodeMenu {
class CheatRegistry : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
  static CheatRegistry *GetSingleton();

  // Event handling
  void RegisterForEvents();
  void OnNewGame();
  void OnPostLoadGame();

  // BSTEventSink
  RE::BSEventNotifyControl ProcessEvent(
      const RE::MenuOpenCloseEvent *a_event,
      RE::BSTEventSource<RE::MenuOpenCloseEvent> *a_eventSource) override;

  // Load all cheat definitions from disk
  void LoadFromDisk();

  // Apply default enabled state for cheats (activates them if startEnabled is
  // true)
  void ApplyDefaultEnabledState();

  // Get a cheat definition by ID (nullptr if not found)
  const CheatDef *GetCheat(const std::string &id) const;

  // Look up which cheat IDs match a given code (normalized, case-insensitive)
  std::vector<std::string> LookupCode(const std::string &code) const;

  // Get all cheat IDs in an exclusive group
  std::vector<std::string> GetCheatsInGroup(const std::string &group) const;

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

  // Explicit sort order from JSON
  std::vector<std::string> m_explicitSortOrder;

  // Re-sort m_orderedIds based on m_explicitSortOrder
  void ApplySortOrder();

  // Code to cheat ID mapping (normalized codes)
  std::unordered_multimap<std::string, std::string> m_codeToIds;

  // Exclusive group to cheat ID mapping
  std::unordered_multimap<std::string, std::string> m_groupToIds;

  // New Game state
  bool m_pendingNewGameCheats = false;
};
} // namespace CheatCodeMenu
