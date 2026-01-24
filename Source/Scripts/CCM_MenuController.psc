ScriptName CCM_MenuController extends Quest
{Cheat Code Menu controller - handles hotkey event and menu loop}

Event OnInit()
	RegisterForModEvent("CheatCodeMenu_OpenMenu", "OnOpenCheatMenu")
	Debug.Trace("[CheatCodeMenu] Controller initialized, listening for menu open event")
EndEvent

Event OnOpenCheatMenu(String eventName, String strArg, Float numArg, Form sender)
	Debug.Trace("[CheatCodeMenu] Menu open event received")
	ShowMenuLoop()
EndEvent

Function ShowMenuLoop()
	Bool keepOpen = True
	Int choice = 0
	String selectedKey = ""
	String cheatInput = ""
	Int cheatState = 0
	Int entryIndex = 0
	
	While keepOpen
		String[] labels = CCM_Native.GetMenuOptionLabels()
		String[] keys = CCM_Native.GetMenuOptionKeys()
		
		UIListMenu listMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
		If listMenu == None
			Debug.Trace("[CheatCodeMenu] ERROR: Could not get UIListMenu!")
			Return
		EndIf
		
		entryIndex = 0
		While entryIndex < labels.Length
			listMenu.AddEntryItem(labels[entryIndex])
			entryIndex += 1
		EndWhile
		
		UIExtensions.OpenMenu("UIListMenu")
		choice = UIExtensions.GetMenuResultInt("UIListMenu")
		
		If choice == -1
			keepOpen = False
		Else
			selectedKey = keys[choice]
			
			If selectedKey == "__enter_code__"
				UIExtensions.InitMenu("UITextEntryMenu")
				UIExtensions.OpenMenu("UITextEntryMenu")
				cheatInput = UIExtensions.GetMenuResultString("UITextEntryMenu")
				If cheatInput != ""
					If CCM_Native.SubmitCode(cheatInput)
						Debug.Notification("Code accepted!")
					Else
						Debug.Notification("Invalid code.")
					EndIf
				EndIf
			Else
				cheatState = CCM_Native.GetCheatState(selectedKey)
				If cheatState == 2
					If CCM_Native.ActivateCheat(selectedKey)
						If CCM_Native.IsTriggerMode(selectedKey)
							Debug.Notification("Cheat executed!")
						ElseIf CCM_Native.IsCheatEnabled(selectedKey)
							Debug.Notification("Cheat enabled!")
						Else
							Debug.Notification("Cheat disabled.")
						EndIf
					Else
						Debug.Notification("Activation failed.")
					EndIf
				Else
					Debug.Notification("This cheat is locked.")
				EndIf
			EndIf
		EndIf
	EndWhile
EndFunction
