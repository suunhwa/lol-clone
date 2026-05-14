@echo off

set EDITOR="C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJECT="C:\Projects\POTENUP-LeagueofLegends\LeagueofLegends.uproject"
set MAP=/Game/Maps/Lv_Lobby

start "Listen Server" %EDITOR% %PROJECT% %MAP%?listen -game -ResX=700 -ResY=580 -WinX=0 -WinY=0 -WINDOWED -log

timeout /t 5 /nobreak > nul

start "Client" %EDITOR% %PROJECT% 127.0.0.1 -game -ResX=700 -ResY=580 -WinX=710 -WinY=0 -WINDOWED -log