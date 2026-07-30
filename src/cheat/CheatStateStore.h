#pragma once

#include "CheatDef.h"
#include <mutex>
#include <string>
#include <unordered_map>

namespace CheatCodeMenu {
class CheatStateStore {
public:
  static CheatStateStore *GetSingleton();

  void Initialize();

  CheatState GetState(const std::string &id) const;

  bool IsToggleEnabled(const std::string &id) const;

  void Unlock(const std::string &id);

  void SetToggleEnabled(const std::string &id, bool enabled);

  bool HasAutoActivated(const std::string &id) const;
  void SetHasAutoActivated(const std::string &id, bool activated);

  void Save(SKSE::SerializationInterface *intfc);
  void Load(SKSE::SerializationInterface *intfc);
  void Revert(SKSE::SerializationInterface *intfc);

private:
  CheatStateStore() = default;
  ~CheatStateStore() = default;

  CheatStateStore(const CheatStateStore &) = delete;
  CheatStateStore(CheatStateStore &&) = delete;
  CheatStateStore &operator=(const CheatStateStore &) = delete;
  CheatStateStore &operator=(CheatStateStore &&) = delete;

  mutable std::mutex m_mutex;
  std::unordered_map<std::string, CheatRuntimeState> m_states;
};
} // namespace CheatCodeMenu
