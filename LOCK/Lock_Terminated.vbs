Set objShell = CreateObject("WScript.Shell")
objShell.Run "wmic process where name='LOCKoff.exe' call terminate", 0, True
Set objShell = Nothing