Scriptname CCMUI_PlayerScript extends ReferenceAlias

Event OnPlayerLoadGame()
	(GetOwningQuest() as CCMUI_Script).Init()
EndEvent
