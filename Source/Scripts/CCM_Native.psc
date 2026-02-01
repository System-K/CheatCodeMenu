ScriptName CCM_Native Hidden
{Native functions exposed by CheatCodeMenu SKSE plugin}

; Returns array of menu option labels for ShowList
; Index 0 is always "Enter Code..."
String[] Function GetMenuOptionLabels() Global Native

; Returns array of menu option keys parallel to labels
; "__enter_code__" for code entry, otherwise cheat ID
String[] Function GetMenuOptionKeys() Global Native

; Validates code and unlocks matching cheats
; Returns true if code was valid and unlocked at least one cheat
Bool Function SubmitCode(String code) Global Native

; Returns cheat state: 0=locked, 1=hidden, 2=unlocked
Int Function GetCheatState(String id) Global Native

; Activates an unlocked cheat
; Returns true if activation succeeded
Bool Function ActivateCheat(String id) Global Native

; For toggle mode cheats, returns current enabled state
Bool Function IsCheatEnabled(String id) Global Native

; Returns true if this cheat is trigger mode (vs toggle mode)
Bool Function IsTriggerMode(String id) Global Native

; Debug: Reloads cheat definitions from disk

; Get a customizable message from the INI configuration
String Function GetMessage(String key) Global Native
