Scriptname CCMUI_Script extends Quest

String sMenu = "Journal Menu"
String sTarget = "_root.QuestJournalFader.Menu_mc"

Bool bOpenCCM

Event OnInit()
	Debug.Trace("[CCMUI] OnInit fired")
	Init()
EndEvent

Function Init()
	Debug.Trace("[CCMUI] Init - Registering for Journal Menu")
	RegisterForMenu(sMenu)
EndFunction

Event OnMenuOpen(String sMenuName)
	Debug.Trace("[CCMUI] OnMenuOpen: " + sMenuName)
	bOpenCCM = False
	String[] args = new String[2]
	args[0] = "CCMButton"
	args[1] = Utility.RandomInt(3000, 4000)
	UI.InvokeStringA(sMenu, sTarget + ".createEmptyMovieClip", args)
	UI.InvokeString(sMenu, sTarget + "." + args[0] + ".loadMovie", "ccmui_inject.swf")
	Debug.Trace("[CCMUI] SWF load invoked")
	RegisterForModEvent("CCMUI_Open", "OnCCMOpen")
	RegisterForModEvent("CCMUI_Debug", "OnCCMDebug")
EndEvent

Event OnCCMDebug(String asEventName, String asStringArg, Float afNumArg, Form akSender)
	Debug.Trace("[CCMUI-AS] " + asStringArg)
EndEvent

Event OnCCMOpen(String asEventName, String asStringArg, Float afNumArg, Form akSender)
	bOpenCCM = True
	UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", sMenu)
EndEvent

Event OnMenuClose(String sMenuName)
	UnregisterForModEvent("CCMUI_Open")
	If bOpenCCM
		SendModEvent("CheatCodeMenu_OpenMenu")
	EndIf
EndEvent
