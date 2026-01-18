ScriptName CCM_MenuController extends Quest
{Cheat Code Menu controller - handles hotkey event and menu loop}

UILIB_1 Property UILib Auto

Event OnInit()
	RegisterForModEvent("CheatCodeMenu_OpenMenu", "OnOpenCheatMenu")
	Debug.Trace("[CheatCodeMenu] Controller initialized, listening for menu open event")
EndEvent

Event OnOpenCheatMenu(String eventName, String strArg, Float numArg, Form sender)
	Debug.Trace("[CheatCodeMenu] Menu open event received")
	ShowMenuLoop()
EndEvent

Function ShowMenuLoop()
	If UILib == None
		Debug.Trace("[CheatCodeMenu] ERROR: UILib property not set!")
		Return
	EndIf
	
	Bool keepOpen = True
	Int choice = 0
	String selectedKey = ""
	String cheatInput = ""
	Int cheatState = 0
	
	While keepOpen
		String[] labels = CCM_Native.GetMenuOptionLabels()
		String[] keys = CCM_Native.GetMenuOptionKeys()
		
		choice = UILib.ShowList("Cheat Codes", labels, 0, 0)
		
		If choice == -1
			keepOpen = False
		Else
			selectedKey = keys[choice]
			
			If selectedKey == "__enter_code__"
				cheatInput = UILib.ShowTextInput("Enter Code", "")
				If cheatInput != ""
					If CCM_Native.SubmitCode(cheatInput)
						UILib.ShowNotification("Code accepted!", "#00FF00")
					Else
						UILib.ShowNotification("Invalid code.", "#FF6666")
					EndIf
				EndIf
			Else
				cheatState = CCM_Native.GetCheatState(selectedKey)
				If cheatState == 2
					If CCM_Native.ActivateCheat(selectedKey)
						If CCM_Native.IsCheatEnabled(selectedKey)
							UILib.ShowNotification("Cheat enabled!", "#00FF00")
						Else
							UILib.ShowNotification("Cheat disabled.", "#FFFF00")
						EndIf
					Else
						UILib.ShowNotification("Activation failed.", "#FF6666")
					EndIf
				Else
					UILib.ShowNotification("This cheat is locked.", "#FF6666")
				EndIf
			EndIf
		EndIf
	EndWhile
EndFunction
