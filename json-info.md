# CheatCodeMenu JSON Schema

This document describes the JSON format for defining cheats in CheatCodeMenu.

## File Location

Place JSON files in: `Data/SKSE/Plugins/CheatCodeMenu/cheats/`

Files must have a `.json` extension. Multiple JSON files are supported.

## Root Object

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `schemaVersion` | integer | **Yes** | Must be `1`. Used for format validation. |
| `cheats` | array | **Yes** | Array of cheat definition objects. |

## Cheat Definition Object

### Required Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `id` | string | **Yes** | Unique identifier for the cheat (e.g., `"sample.godmode"`). Used internally. |
| `displayName` | string | **Yes** | Name shown in menu when unlocked (e.g., `"God Mode"`). |
| `code` | string | **Yes** | Code phrase to unlock the cheat (e.g., `"iddqd"`). Case-insensitive, whitespace-trimmed. |
| `defaultState` | string | **Yes** | Initial state: `"locked"`, `"hidden"`, or `"unlocked"`. |
| `mode` | string | **Yes** | Activation mode: `"trigger"` or `"toggle"`. |

### Optional Fields

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `lockedName` | string | No | `"?????"` | Name shown in menu when locked (e.g., `"??? Divine Power"`). |
| `actions` | array | No | `[]` | Actions to execute (for `trigger` mode, or general actions). |
| `enableActions` | array | No | `[]` | Actions when toggling ON (only for `toggle` mode). |
| `disableActions` | array | No | `[]` | Actions when toggling OFF (only for `toggle` mode). |

## State Values

- **`"locked"`**: Visible in menu with `lockedName`, cannot be activated until code is entered
- **`"hidden"`**: Not visible in menu until code is entered
- **`"unlocked"`**: Starts unlocked and visible with `displayName`

## Mode Values

- **`"trigger"`**: Execute `actions` each time activated
- **`"toggle"`**: Flip between enabled/disabled states, running `enableActions`/`disableActions`

## Action Object

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | **Yes** | Action type. Currently only `"console"` is supported. |
| `params` | object | **Yes** | Type-specific parameters. |

### Console Action

For `"console"` type actions:

```json
{
  "type": "console",
  "params": {
    "command": "tgm"
  }
}
```

The `command` parameter is required and should contain a valid Skyrim console command.

## Complete Example

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
        {
          "type": "console",
          "params": { "command": "tgm" }
        }
      ],
      "disableActions": [
        {
          "type": "console",
          "params": { "command": "tgm" }
        }
      ]
    },
    {
      "id": "mymod.richquick",
      "displayName": "Get Rich Quick (+10000 Gold)",
      "code": "showmethemoney",
      "defaultState": "unlocked",
      "mode": "trigger",
      "actions": [
        {
          "type": "console",
          "params": { "command": "player.additem f 10000" }
        }
      ]
    },
    {
      "id": "mymod.secret",
      "displayName": "Secret Power",
      "code": "konami",
      "defaultState": "hidden",
      "mode": "trigger",
      "actions": [
        {
          "type": "console",
          "params": { "command": "player.setav health 9999" }
        }
      ]
    }
  ]
}
```

## Notes

- **Multiple cheats can share the same code** - all matching cheats will unlock when the code is entered
- **Code comparison is case-insensitive** and ignores leading/trailing whitespace
- **Invalid files are logged and skipped** - they don't prevent other files from loading
- **Duplicate IDs are rejected** - only the first cheat with a given ID is loaded
- **Action arrays execute sequentially** - multiple commands run in order
