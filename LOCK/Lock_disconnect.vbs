MsgBox "lock disconnected",vbinformation,"LOCK"
Set objShell = CreateObject("WScript.Shell")
objShell.Run "wmic process where name='LOCKser.exe' call terminate", 0, True
Set objShell = Nothing
Set objShell = CreateObject("WScript.Shell")
objShell.Run "wmic process where name='LOCKon.exe' call terminate", 0, True
Set objShell = Nothing
Set objShell = CreateObject("WScript.Shell")
objShell.Run "LOCKoff.exe", 1, False
Set objShell = Nothing