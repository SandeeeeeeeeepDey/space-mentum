@echo off
REM ==============================================================================
REM SpaceMentum — One-Click Windows x64 Shipping Standalone Package Batch Runner
REM ==============================================================================

set SCRIPT_DIR=%~dp0
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%PackageSpaceMentumWin64.ps1" %*
pause
