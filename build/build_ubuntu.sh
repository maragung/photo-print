#!/bin/bash
set -e

# ===== CONFIGURATION =====
TARGET_ARCH=${TARGET_ARCH:-"native"}
OUTPUT_FILE=${OUTPUT_FILE:-"PhotoPrinter"}
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "🔨 Building for: $TARGET_ARCH"
echo "📁 Project Root: $PROJECT_ROOT"

# ===== SETUP COMPILER & FLAGS =====
case $TARGET_ARCH in
  x86_64|x64)
    echo "⚙️  Configuring for x86_64..."
    export CC=gcc
    export CXX=g++
    export CFLAGS="-m64 -O2"
    export CXXFLAGS="-m64 -O2 -std=c++11"
    export LDFLAGS="-m64"
    ;;
  
  i686|i386|x86)
    echo "⚙️  Configuring for i686 (32-bit)..."
    export CC=gcc
    export CXX=g++
    export CFLAGS="-m32 -O2"
    export CXXFLAGS="-m32 -O2 -std=c++11"
    export LDFLAGS="-m32"
    ;;
  
  armv7|arm)
    echo "⚙️  Configuring for ARMv7..."
    export CC=arm-linux-gnueabihf-gcc
    export CXX=arm-linux-gnueabihf-g++
    export CFLAGS="-march=armv7-a -mfpu=neon -O2"
    export CXXFLAGS="-march=armv7-a -mfpu=neon -O2 -std=c++11"
    export LDFLAGS=""
    ;;
  
  aarch64|arm64)
    echo "⚙️  Configuring for ARM64..."
    export CC=aarch64-linux-gnu-gcc
    export CXX=aarch64-linux-gnu-g++
    export CFLAGS="-O2"
    export CXXFLAGS="-O2 -std=c++11"
    export LDFLAGS=""
    ;;
  
  native)
    echo "⚙️  Configuring for native build..."
    export CC=gcc
    export CXX=g++
    export CFLAGS="-O2"
    export CXXFLAGS="-O2 -std=c++11"
    export LDFLAGS=""
    ;;
  
  *)
    echo "❌ Unknown target architecture: $TARGET_ARCH"
    exit 1
    ;;
esac

# ===== BUILD PROCESS =====
echo "🔨 Starting compilation..."

cd "$PROJECT_ROOT"

# Compile main.cpp
echo "📝 Compiling main.cpp..."
$CXX $CXXFLAGS -c main.cpp -o main.o

# Link with resource.o and system libraries
echo "🔗 Linking binary..."
$CXX $LDFLAGS main.o "$SCRIPT_DIR/resource.o" \
  -o "$SCRIPT_DIR/$OUTPUT_FILE" \
  -lws2_32 -lgdiplus -lcomctl32 -lstdc++ -lm

# Verify output
if [ -f "$SCRIPT_DIR/$OUTPUT_FILE" ]; then
  echo "✅ Build successful!"
  echo "📦 Output: $SCRIPT_DIR/$OUTPUT_FILE"
  ls -lh "$SCRIPT_DIR/$OUTPUT_FILE"
else
  echo "❌ Build failed - output file not found"
  exit 1
fi
