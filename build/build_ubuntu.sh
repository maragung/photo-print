#!/bin/bash
cd "$(dirname "$0")"

# Cross-compilation build script for deploying the Windows C++ Application from inside Ubuntu/Linux using MinGW-w64

echo "Compiling Photo Printer natively for Windows (x86_64) from Ubuntu Linux..."

# Check if MinGW-w64 is installed
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Error: MinGW-w64 compiler chain is not installed!"
    echo "Please install it first by executing: sudo apt-get update && sudo apt-get install mingw-w64"
    exit 1
fi

echo "Compiling native Windows UI resources (icons)..."
x86_64-w64-mingw32-windres ../resource.rc -o resource.o
if [ $? -ne 0 ]; then
    echo "Resource compilation failed!"
    exit 1
fi

echo "Compiling C++ Win32 Source Matrix..."
x86_64-w64-mingw32-g++ ../main.cpp resource.o -o PhotoPrinter.exe \
    -std=c++11 \
    -mwindows \
    -municode \
    -static \
    -static-libgcc \
    -static-libstdc++ \
    -lgdiplus -lgdi32 -lcomctl32 -lcomdlg32 -lole32

if [ $? -ne 0 ]; then
    echo "Build Phase completed with critical errors. Please check the logs."
    exit 1
else
    echo "Success! Binary structurally compiled: build/PhotoPrinter.exe"
fi
