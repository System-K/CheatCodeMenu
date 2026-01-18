#pragma once

#include <SKSE/SKSE.h>

namespace CheatCodeMenu::Serialization {
constexpr std::uint32_t kSerializationVersion = 1;
constexpr std::uint32_t kCheatData = 'CHTS';

void SaveCallback(SKSE::SerializationInterface *intfc);
void LoadCallback(SKSE::SerializationInterface *intfc);
void RevertCallback(SKSE::SerializationInterface *intfc);

void Register();
} // namespace CheatCodeMenu::Serialization
