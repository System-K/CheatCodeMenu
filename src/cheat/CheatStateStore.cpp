#include "CheatStateStore.h"
#include "CheatRegistry.h"
#include "ConfigManager.h"

namespace CheatCodeMenu {
CheatStateStore *CheatStateStore::GetSingleton() {
  static CheatStateStore singleton;
  return &singleton;
}

void CheatStateStore::Initialize() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_states.clear();

  auto *registry = CheatRegistry::GetSingleton();
  for (const auto &id : registry->GetAllIds()) {
    const auto *def = registry->GetCheat(id);
    if (def) {
      CheatRuntimeState state;
      state.state = def->defaultState;
      state.toggleEnabled = false;
      state.hasAutoActivated = false;
      m_states[id] = state;
    }
  }

  logger::info("CheatStateStore initialized with {} cheats", m_states.size());
}

CheatState CheatStateStore::GetState(const std::string &id) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_states.find(id);
  if (it != m_states.end()) {
    if (ConfigManager::GetSingleton()->GetUnlockAllCodes()) {
      return CheatState::Unlocked;
    }
    return it->second.state;
  }
  return CheatState::Locked;
}

bool CheatStateStore::IsToggleEnabled(const std::string &id) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_states.find(id);
  if (it != m_states.end()) {
    return it->second.toggleEnabled;
  }
  return false;
}

void CheatStateStore::Unlock(const std::string &id) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_states.find(id);
  if (it != m_states.end()) {
    if (it->second.state != CheatState::Unlocked) {
      it->second.state = CheatState::Unlocked;
      logger::info("Cheat '{}' unlocked", id);
    }
  }
}

void CheatStateStore::SetToggleEnabled(const std::string &id, bool enabled) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_states.find(id);
  if (it != m_states.end()) {
    it->second.toggleEnabled = enabled;
  }
}

bool CheatStateStore::HasAutoActivated(const std::string &id) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_states.find(id);
  if (it != m_states.end()) {
    return it->second.hasAutoActivated;
  }
  return false;
}

void CheatStateStore::SetHasAutoActivated(const std::string &id,
                                          bool activated) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_states.find(id);
  if (it != m_states.end()) {
    it->second.hasAutoActivated = activated;
  }
}

void CheatStateStore::Save(SKSE::SerializationInterface *intfc) {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::vector<std::pair<std::string, CheatRuntimeState>> toSave;
  auto *registry = CheatRegistry::GetSingleton();

  for (const auto &[id, state] : m_states) {
    const auto *def = registry->GetCheat(id);
    if (!def)
      continue;

    if (state.state != def->defaultState || state.toggleEnabled ||
        state.hasAutoActivated) {
      toSave.emplace_back(id, state);
    }
  }

  uint32_t count = static_cast<uint32_t>(toSave.size());
  intfc->WriteRecordData(&count, sizeof(count));

  for (const auto &[id, state] : toSave) {
    uint32_t idLen = static_cast<uint32_t>(id.length());
    intfc->WriteRecordData(&idLen, sizeof(idLen));
    intfc->WriteRecordData(id.data(), idLen);

    uint8_t stateVal = static_cast<uint8_t>(state.state);
    intfc->WriteRecordData(&stateVal, sizeof(stateVal));

    uint8_t toggleVal = state.toggleEnabled ? 1 : 0;
    intfc->WriteRecordData(&toggleVal, sizeof(toggleVal));

    uint8_t autoActiveVal = state.hasAutoActivated ? 1 : 0;
    intfc->WriteRecordData(&autoActiveVal, sizeof(autoActiveVal));
  }

  logger::info("Saved {} cheat states to co-save", count);
}

void CheatStateStore::Load(SKSE::SerializationInterface *intfc) {
  std::lock_guard<std::mutex> lock(m_mutex);

  uint32_t count = 0;
  if (!intfc->ReadRecordData(&count, sizeof(count))) {
    logger::error("Failed to read cheat state count");
    return;
  }

  auto *registry = CheatRegistry::GetSingleton();
  uint32_t loaded = 0;
  uint32_t skipped = 0;

  for (uint32_t i = 0; i < count; ++i) {
    uint32_t idLen = 0;
    if (!intfc->ReadRecordData(&idLen, sizeof(idLen)))
      break;

    std::string id(idLen, '\0');
    if (!intfc->ReadRecordData(id.data(), idLen))
      break;

    uint8_t stateVal = 0;
    if (!intfc->ReadRecordData(&stateVal, sizeof(stateVal)))
      break;

    uint8_t toggleVal = 0;
    if (!intfc->ReadRecordData(&toggleVal, sizeof(toggleVal)))
      break;

    uint8_t autoActiveVal = 0;
    if (!intfc->ReadRecordData(&autoActiveVal, sizeof(autoActiveVal)))
      break;

    if (!registry->GetCheat(id)) {
      skipped++;
      continue;
    }

    auto it = m_states.find(id);
    if (it != m_states.end()) {
      it->second.state = static_cast<CheatState>(stateVal);
      it->second.toggleEnabled = (toggleVal != 0);
      it->second.hasAutoActivated = (autoActiveVal != 0);
      loaded++;
    }
  }

  logger::info(
      "Loaded {} cheat states from co-save ({} skipped - no longer exist)",
      loaded, skipped);
}

void CheatStateStore::Revert(SKSE::SerializationInterface *) {
  Initialize();
  logger::info("CheatStateStore reverted to defaults");
}
} // namespace CheatCodeMenu
