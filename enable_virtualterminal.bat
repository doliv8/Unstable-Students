:: enable colors in terminal enabling Virtual Terminal
reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1 /f
echo "VirtualTerminal abilitato!"
pause