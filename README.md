# Cheat Code Menu

An SKSE plugin framework for in-game cheat code menus, powered by UIExtensions.

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

## How to Use:

In order to create your own cheats, you will need to place a .json file in  
``` 
Data/SKSE/Plugins/CheatCodeMenu/cheats
```
JSON file information can be found in [json-info.md](json-info.md). A sample json can be found in Data/SKSE/Plugins/CheatCodeMenu/cheats/sample_cheats.json.

Let's examine some potential use cases:

### **Creating a Cheat Pack:**

In order to create your own cheat pack, you need a mod of some sort with your custom effects (either an SKSE plugin, an esp file, or a combination of both). For cheats contained within an esp file, you need a way to easily trigger them via a console command. For example, in my cheat pack, I primarily use a mixture of variables (set globalvariablecheat to 1) and quests (startquest cheatpackquest). For cheats contained within an SKSE plugin, you need an event listener so that the cheat menu can pass an event to the plugin. In general, I recommend using the numArg option as an enable/disable argument. See the github repos for examples on doing so.

Once you have all of your cheats, make sure that, if you want them in a specific order, you define it in the json (alternatively, they will be listed in the order they are added in the json file - not too big of a deal if you prefer to do it that way).

Feel free to play around with some of the other options - you can set custom hidden text (potentially hints as to what the code might be), have cheats enabled by default, etc.

Additionally, you may wish to play around with alternative methods of sharing the cheat codes. I've simply included them in the mod page for this mod, but you could, for example, add notes around the game that hint at the codes. If there's demand, I could also add a papyrus function that unlocks a code.

### **Adding Cheats to an Existing Mod:**

As there's no need for your mod to require this one, you can add cheats to your mod that are only available if this mod is installed. Simply set up the infrastructure for it, like above, and then ship the mod with the json included. If CHEAT CODES is installed, your cheats will be added to the menu. If not, they simply won't appear to the user (or the user can use console commands to trigger them). This is a fun way to add easter eggs to your mod without needing to add any additional requirements.

### **Modlist Authors:**

For modlist authors, there are two potential uses: a custom cheat code mod (see above), or you can use this as an easily-accessible menu for users. For this use case, there are a few things to note:

- All text can be defined in  
``` 
Data/SKSE/Plugins/CheatCodeMenu.ini
```

- All codes can be set to unlocked by default (meaning it simply shows up like a list of options)  
- When all codes are unlocked, the "Enter Code" prompt disappears (meaning it won't look like there should be codes the user can input)  
- The menu entry is compatible with all UI replacers  

For example, let's pretend that I have a modlist called "Super Skyrim". I can adjust the ini like so:

``` 
[Menu]
; Text displayed on the System menu button
ButtonText = SUPER SKYRIM

[Messages]
CodeAccepted = Code accepted!
InvalidCode = Invalid code.
CheatExecuted = Setting toggled.
CheatEnabled = Setting enabled.
CheatDisabled = Setting disabled.
ActivationFailed = Activation failed.
CheatLocked = This cheat is locked.
```

Note that I didn't bother adjusting the CodeAccepted, InvalidCode, or CheatLocked values, as the user won't ever see them in this case. Now, the user will see a SUPER SKYRIM entry in the system menu. When they click it, they'd see whatever options we want. For example, they might see something like:

- Story Mode [ON]  
- Show Compass [ON]  
- Disable Spiders [OFF]  
- Essential Followers [OFF]  
- etc.  
- etc.  

This allows you to have a quick gameplay options menu in a user-friendly location.

## Credits

- Template based on CLibDT and [PrismaUI](https://github.com/PrismaUI-SKSE/PrismaUI-Example-Plugin)
- [UIExtensions](https://www.nexusmods.com/skyrimspecialedition/mods/17561) by expired6978 - UI menu components
- [Dragonborn's Bestiary - Show in UI](https://www.nexusmods.com/skyrimspecialedition/mods/161050) - Pause menu injection technique
- [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) - SKSE framework
- [nlohmann/json](https://github.com/nlohmann/json) - JSON parsing

## License

LGPL-3.0
