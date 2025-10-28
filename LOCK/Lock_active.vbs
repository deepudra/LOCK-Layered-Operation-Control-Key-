MsgBox "lock connected",vbinformation,"LOCK"
Set objShell = CreateObject("WScript.Shell")
objShell.Run "LOCKser.exe", 1, False
Set objShell = Nothing
Set objShell = CreateObject("WScript.Shell")
objShell.Run "wmic process where name='LOCKoff.exe' call terminate", 0, True
Set objShell = Nothing
Set objShell = CreateObject("WScript.Shell")
objShell.Run "LOCKon.exe", 1, False
Set objShell = Nothing