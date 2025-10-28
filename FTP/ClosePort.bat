@echo off
netsh advfirewall firewall delete rule name="Open FTP Port 2121"
echo FTP port 2121 is now closed.
pause