#pragma once

#include "CheatDef.h"
#include <mutex>
#include <string>
#include <unordered_map>

namespace CheatCodeMenu {
class CheatStateStore {
public:
  static CheatStateStore *GetSingleton();

  // Initialize all states from registry defaults
  void Initialize();

  // Get current state of a cheat
  CheatState GetState(const std::string &id) const;

  // Check if toggle is enabled (only meaningful for toggle mode)
  bool IsToggleEnabled(const std::string &id) const;

  // Unlock a cheat (transitions locked/hidden -> unlocked)
  void Unlock(const std::string &id);

  // Set toggle enabled state
  void SetToggleEnabled(const std::string &id, bool enabled);

  // Check/Set if a cheat has been auto-activated for this save
  bool HasAutoActivated(const std::string &id) const;
  void SetHasAutoActivated(const std::string &id, bool activated);

  // SKSE Serialization
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
