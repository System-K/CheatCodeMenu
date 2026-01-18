#include "Serialization.h"
#include "cheat/CheatStateStore.h"

namespace CheatCodeMenu::Serialization {
void SaveCallback(SKSE::SerializationInterface *intfc) {
  if (!intfc->OpenRecord(kCheatData, kSerializationVersion)) {
    logger::error("Failed to open serialization record for save");
    return;
  }

  CheatStateStore::GetSingleton()->Save(intfc);
}

void LoadCallback(SKSE::SerializationInterface *intfc) {
  std::uint32_t type;
  std::uint32_t version;
  std::uint32_t length;

  while (intfc->GetNextRecordInfo(type, version, length)) {
    if (type == kCheatData) {
      if (version != kSerializationVersion) {
        logger::warn(
            "Serialization version mismatch (got {}, expected {}), skipping",
            version, kSerializationVersion);
        continue;
      }

      CheatStateStore::GetSingleton()->Load(intfc);
    }
  }
}

void RevertCallback(SKSE::SerializationInterface *intfc) {
  CheatStateStore::GetSingleton()->Revert(intfc);
}

void Register() {
  auto *serialization = SKSE::GetSerializationInterface();
  if (!serialization) {
    logger::error("Failed to get serialization interface");
    return;
  }

  serialization->SetUniqueID('CCM_');
  serialization->SetSaveCallback(SaveCallback);
  serialization->SetLoadCallback(LoadCallback);
  serialization->SetRevertCallback(RevertCallback);

  logger::info("Registered serialization callbacks");
}
} // namespace CheatCodeMenu::Serialization
