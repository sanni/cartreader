@echo off
powershell Install-Module -Name ps2exe -Scope CurrentUser
powershell Invoke-ps2exe -inputFile "oscr_tool.ps1" -outputFile "oscr_tool.exe" -iconFile "icon.ico"
pause