#pragma once

#include <SKSE/SKSE.h>

namespace CheatCodeMenu::PapyrusAPI {
// Register all native functions with the Papyrus VM
bool Register(RE::BSScript::IVirtualMachine *vm);
} // namespace CheatCodeMenu::PapyrusAPI
