# Cheat Code Menu

An SKSE plugin framework for in-game cheat code menus, powered by UIExtensions.

## Features

- **JSON-driven cheats**: Define cheats in extensible JSON files
- **Code unlock system**: Enter codes to unlock hidden/locked cheats
- **Console command actions**: Execute console commands on cheat activation
- **Toggle support**: Cheats can be trigger (one-shot) or toggle (on/off)
- **Persistent state**: Unlock states persist across save/load via SKSE co-save
- **Configurable hotkey**: Set menu hotkey via INI file

## Requirements

- Skyrim SE/AE (1.5.97+ / 1.6.x)
- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [UIExtensions](https://www.nexusmods.com/skyrimspecialedition/mods/17561) by expired6978

## Installation

1. Install requirements above
2. Copy `CheatCodeMenu.dll` to `Data/SKSE/Plugins/`
3. Copy `CheatCodeMenu.ini` to `Data/SKSE/Plugins/`
4. Copy `CheatCodeMenu.esp` to `Data/`
5. Copy compiled scripts to `Data/Scripts/`

## Configuration

Edit `Data/SKSE/Plugins/CheatCodeMenu.ini`:

```ini
[Hotkey]
; DirectX scan code (default: F9 = 0x43)
ScanCode = 0x43

[Debug]
VerboseLogging = 0
```

## Creating Cheats

Place JSON files in `Data/SKSE/Plugins/CheatCodeMenu/cheats/`:

```json
{
  "schemaVersion": 1,
  "cheats": [
    {
      "id": "mymod.godmode",
      "displayName": "God Mode",
      "lockedName": "??? Divine Power",
      "code": "iddqd",
      "defaultState": "locked",
      "mode": "toggle",
      "enableActions": [
        { "type": "console", "params": { "command": "tgm" } }
      ],
      "disableActions": [
        { "type": "console", "params": { "command": "tgm" } }
      ]
    }
  ]
}
```

### Cheat Properties

| Property | Required | Description |
|----------|----------|-------------|
| `id` | Yes | Unique identifier |
| `displayName` | Yes | Label when unlocked |
| `lockedName` | No | Label when locked (default: "?????") |
| `code` | Yes | Code to unlock (case-insensitive) |
| `defaultState` | Yes | `"locked"`, `"hidden"`, or `"unlocked"` |
| `mode` | Yes | `"trigger"` or `"toggle"` |
| `actions` | No | Actions for trigger mode |
| `enableActions` | No | Actions when toggle enabled |
| `disableActions` | No | Actions when toggle disabled |

### Action Types

| Type | Params | Description |
|------|--------|-------------|
| `console` | `{ "command": "..." }` | Execute console command |

## Building

Requires XMake 2.8.2+ and MSVC C++23 compiler.

```bash
xmake build
```

## Credits

- Template based on CLibDT and [PrismaUI](https://github.com/PrismaUI-SKSE/PrismaUI-Example-Plugin)
- [UIExtensions](https://www.nexusmods.com/skyrimspecialedition/mods/17561) by expired6978 - UI menu components
- [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) - SKSE framework
- [nlohmann/json](https://github.com/nlohmann/json) - JSON parsing

## License

GPL-3.0