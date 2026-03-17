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
6. Copy `CheatCodeMenu.swf` to `Data/Interface/`

## Flash Compilation
1. Any edits to CCMCallbacks.as will need to be copied to frame 1 of the fla file.
2. Compile using Flash CS6

## Configuration

Edit `Data/SKSE/Plugins/CheatCodeMenu.ini`:

```ini
[Hotkey]
; DirectX scan code for menu hotkey
; Insert = 0xD2 (67 decimal)
; See: https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-6.0/aa299374(v=vs.60)
ScanCode = -1

[Menu]
; Text displayed on the System menu button
ButtonText = CHEAT CODES

[Debug]
; Enable verbose logging (0 = off, 1 = on)
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
| `startEnabled` | No | Auto-activate on startup without changing locked/hidden/unlocked state |
| `mode` | Yes | `"trigger"` or `"toggle"` |
| `actions` | No | Actions for trigger mode |
| `enableActions` | No | Actions when toggle enabled |
| `disableActions` | No | Actions when toggle disabled |

### Startup Activation

`startEnabled` controls activation, not discovery. When it is `true`, the cheat is auto-activated once per save on startup/new game, but its `defaultState` is left alone.

That means a cheat can start active while still being `locked` or `hidden`. For toggle cheats, this is intentional: the effect can be forced on until the player discovers the code, at which point it becomes unlockable and can be turned off from the menu. If you want a startup-enabled cheat to be immediately visible and switchable, use `defaultState: "unlocked"` together with `startEnabled: true`.

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
- [Dragonborn's Bestiary - Show in UI](https://www.nexusmods.com/skyrimspecialedition/mods/161050) - Pause menu injection technique
- [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) - SKSE framework
- [nlohmann/json](https://github.com/nlohmann/json) - JSON parsing

## License

LGPL-3.0
