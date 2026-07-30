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

  void RegisterForEvents();
  void OnNewGame();
  void OnPostLoadGame();

  RE::BSEventNotifyControl ProcessEvent(
      const RE::MenuOpenCloseEvent *a_event,
      RE::BSTEventSource<RE::MenuOpenCloseEvent> *a_eventSource) override;

  void LoadFromDisk();

  void ApplyDefaultEnabledState();

  const CheatDef *GetCheat(const std::string &id) const;

  std::vector<std::string> LookupCode(const std::string &code) const;

  std::vector<std::string> GetCheatsInGroup(const std::string &group) const;

  const std::vector<std::string> &GetAllIds() const { return m_orderedIds; }

  size_t GetCount() const { return m_cheats.size(); }

  void Clear();

private:
  CheatRegistry() = default;
  ~CheatRegistry() = default;

  CheatRegistry(const CheatRegistry &) = delete;
  CheatRegistry(CheatRegistry &&) = delete;
  CheatRegistry &operator=(const CheatRegistry &) = delete;
  CheatRegistry &operator=(CheatRegistry &&) = delete;

  bool LoadFile(const std::filesystem::path &filepath);

  bool ParseCheat(const nlohmann::json &j, CheatDef &outCheat,
                  const std::string &filename);

  bool ParseActions(const nlohmann::json &j, std::vector<CheatAction> &outActions,
                    const std::string &filename, const std::string &cheatId,
                    const char *fieldName);

  std::unordered_map<std::string, CheatDef> m_cheats;

  std::vector<std::string> m_orderedIds;

  std::vector<std::string> m_explicitSortOrder;

  void ApplySortOrder();

  std::unordered_multimap<std::string, std::string> m_codeToIds;

  std::unordered_multimap<std::string, std::string> m_groupToIds;

  bool m_pendingNewGameCheats = false;
};
} // namespace CheatCodeMenu
