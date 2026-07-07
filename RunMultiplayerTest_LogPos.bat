@echo off
REM Launcher: host + client windows on the top half (half-width, FHD 16:9),
REM their log consoles docked directly underneath.
REM All logic lives in RunMultiplayerTest_LogPos.ps1 (robust new-window detection).
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0RunMultiplayerTest_LogPos.ps1"
