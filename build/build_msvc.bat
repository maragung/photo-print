@echo off
cd /d "%~dp0"
rc /fo resource.res ../resource.rc
cl /EHsc ../main.cpp resource.res /link /SUBSYSTEM:WINDOWS /OUT:PhotoPrinter.exe gdiplus.lib gdi32.lib comctl32.lib comdlg32.lib ole32.lib user32.lib
