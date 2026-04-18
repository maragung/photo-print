@echo off
cd /d "%~dp0"
echo Building with g++ from build directory...

windres ../resource.rc -o resource.o
g++ ../main.cpp resource.o -o PhotoPrinter.exe -std=c++11 -mwindows -lgdiplus -lgdi32 -lcomctl32 -lcomdlg32 -lole32

if %errorlevel% neq 0 (
    echo Build failed.
) else (
    echo Build successful: build\PhotoPrinter.exe
)
pause
