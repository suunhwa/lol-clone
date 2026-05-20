@echo off

set "EDITOR=C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=C:\Projects\POTENUP-LeagueofLegends\LeagueofLegends.uproject"
set "MAP=/Game/Maps/Lv_Lobby"

start "" ^
"%EDITOR%" ^
"%PROJECT%" ^
%MAP%?listen ^
-game -WINDOWED ^
-ResX=700 -ResY=580 ^
-WinX=0 -WinY=0 ^
-log