# CheatCodeMenu JSON Schema

This document describes the JSON format for defining cheats in CheatCodeMenu.

## File Location

Place JSON files in: `Data/SKSE/Plugins/CheatCodeMenu/cheats/`

Files must have a `.json` extension. Multiple JSON files are supported.

## Root Object

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `schemaVersion` | integer | **Yes** | Must be `1`. Used for format validation. |
| `sortOrder` | array | No | Array of strings (cheat IDs) defining specific menu order. |
| `cheats` | array | **Yes** | Array of cheat definition objects. |

## Cheat Definition Object

### Required Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `id` | string | **Yes** | Unique identifier for the cheat (e.g., `"sample.godmode"`). Used internally. |
| `displayName` | string | **Yes** | Name shown in menu when unlocked (e.g., `"God Mode"`). |
| `code` | string | Conditional | Single code phrase to unlock (e.g., `"iddqd"`). Either `code` or `codes` is required. |
| `codes` | array | Conditional | Array of valid code phrases. Either `code` or `codes` is required. |
| `defaultState` | string | **Yes** | Initial state: `"locked"`, `"hidden"`, or `"unlocked"`. |
| `mode` | string | **Yes** | Activation mode: `"trigger"` or `"toggle"`. |

### Optional Fields

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `lockedName` | string | No | `"?????"` | Name shown in menu when locked (e.g., `"??? Divine Power"`). |
| `startEnabled` | boolean | No | `false` | If `true`, cheat is automatically activated once per save on startup/new game. This does not change `defaultState`. |
| `exclusiveGroup` | string | No | - | Group name for mutual exclusivity. Only one cheat per group can be active (toggle mode only). |
| `actions` | array | No | `[]` | Actions to execute (for `trigger` mode, or general actions). |
| `enableActions` | array | No | `[]` | Actions when toggling ON (only for `toggle` mode). |
| `disableActions` | array | No | `[]` | Actions when toggling OFF (only for `toggle` mode). |

## State Values

- **`"locked"`**: Visible in menu with `lockedName`, cannot be activated until code is entered
- **`"hidden"`**: Not visible in menu until code is entered
- **`"unlocked"`**: Starts unlocked and visible with `displayName`

`startEnabled` is independent from these state values. A cheat may begin active while still `locked` or `hidden`, which allows authors to force an effect on until the code is discovered.

## Mode Values

- **`"trigger"`**: Execute `actions` each time activated
- **`"toggle"`**: Flip between enabled/disabled states, running `enableActions`/`disableActions`

## Action Object

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | **Yes** | Action type: `"console"` or `"modevent"`. |
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

### ModEvent Action

For `"modevent"` type actions (sends SKSE mod events for inter-plugin communication):

```json
{
  "type": "modevent",
  "params": {
    "eventName": "MyPlugin_ToggleFeature",
    "strArg": "optional_string",
    "numArg": 1.0
  }
}
```

| Param | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `eventName` | string | **Yes** | - | Name of the SKSE mod event to send. |
| `strArg` | string | No | `""` | String argument passed with the event. |
| `numArg` | number | No | `0.0` | Numeric argument passed with the event. |

See the [Integration Guide](#integration-guide) for how other SKSE plugins can listen for these events.

## Complete Example

```json
{
  "schemaVersion": 1,
  "sortOrder": ["mymod.godmode", "mymod.immortal"],
  "cheats": [
    {
      "id": "mymod.godmode",
      "displayName": "God Mode",
      "lockedName": "??? Divine Power",
      "codes": ["iddqd", "godmode"],
      "defaultState": "locked",
      "mode": "toggle",
      "exclusiveGroup": "immortality",
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
      "id": "mymod.immortal",
      "displayName": "Immortal Mode",
      "lockedName": "??? Undying",
      "code": "tim",
      "defaultState": "locked",
      "mode": "toggle",
      "exclusiveGroup": "immortality",
      "enableActions": [
        {
          "type": "console",
          "params": { "command": "tim" }
        }
      ],
      "disableActions": [
        {
          "type": "console",
          "params": { "command": "tim" }
        }
      ]
    },
    {
      "id": "mymod.richquick",
      "displayName": "Get Rich Quick (+10000 Gold)",
      "code": "showmethemoney",
      "defaultState": "unlocked",
      "startEnabled": true,
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

- **Multiple codes per cheat** - use `codes` array to allow multiple code phrases to unlock the same cheat
- **Multiple cheats can share the same code** - all matching cheats will unlock when the code is entered
- **Exclusive groups** - toggle cheats in the same `exclusiveGroup` are mutually exclusive (enabling one disables others)
- **`startEnabled` does not unlock cheats** - locked/hidden cheats can auto-activate and remain unavailable in the menu until discovered
- **Code comparison is case-insensitive** and ignores leading/trailing whitespace
- **Invalid files are logged and skipped** - they don't prevent other files from loading
- **Duplicate IDs are rejected** - only the first cheat with a given ID is loaded
- **Action arrays execute sequentially** - multiple commands run in order

## Integration Guide

### For SKSE Plugin Developers

To integrate your SKSE plugin with CheatCodeMenu, listen for SKSE mod events:

#### 1. Create an Event Sink

```cpp
class ModEventSink : public RE::BSTEventSink<SKSE::ModCallbackEvent> {
public:
  static ModEventSink* GetSingleton() {
    static ModEventSink singleton;
    return &singleton;
  }

  RE::BSEventNotifyControl ProcessEvent(
      const SKSE::ModCallbackEvent* a_event,
      RE::BSTEventSource<SKSE::ModCallbackEvent>*) override {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    // Check for your event name
    if (a_event->eventName == "MyPlugin_Toggle") {
      // Handle the event
      // a_event->strArg contains the string argument
      // a_event->numArg contains the numeric argument
      MyPlugin::Toggle();
    }

    return RE::BSEventNotifyControl::kContinue;
  }

private:
  ModEventSink() = default;
};
```

#### 2. Register the Event Sink

In your plugin's `kDataLoaded` message handler:

```cpp
void MessageHandler(SKSE::MessagingInterface::Message* msg) {
  if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
    auto* eventSource = SKSE::GetModCallbackEventSource();
    if (eventSource) {
      eventSource->AddEventSink(ModEventSink::GetSingleton());
    }
  }
}
```

#### 3. Define Your Cheat JSON

```json
{
  "id": "myplugin.toggle",
  "displayName": "My Plugin Toggle",
  "code": "mycode",
  "defaultState": "unlocked",
  "mode": "toggle",
  "enableActions": [
    { "type": "modevent", "params": { "eventName": "MyPlugin_Toggle", "numArg": 1 } }
  ],
  "disableActions": [
    { "type": "modevent", "params": { "eventName": "MyPlugin_Toggle", "numArg": 0 } }
  ]
}
```

### Using numArg for Toggle State

Use `numArg` to differentiate between enable/disable:
- `"numArg": 1` for enable actions
- `"numArg": 0` for disable actions

Check the value in your event handler to determine the action.
