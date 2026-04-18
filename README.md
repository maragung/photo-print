# Photo Print 

A high-performance, lightweight, natively-compiled Windows Desktop application entirely built using the raw Win32 API and GDI+ matrices. Designed by Maragung, this tool allows users to automatically compute physical photo print layouts, actively track grids, perform real-time visual cropping/rotations, and seamlessly inject native rendering payloads directly to Windows Driver-authenticated Physical Printers.

## Key Features

- **Blazing Fast Native C++:** Zero heavy Electron shells or dependencies. The entire GUI framework binds optimally to Windows memory utilizing hardware-accelerated Double-Buffering architectures designed to prevent screen-tearing.
- **Smart Matrix Calculation:** Choose your Photo sizes (2x3, 4R, custom ID Card Dimensions) mapped directly against international Paper scales (A4, A3, letter). The background layout engine automatically deduces maximum physical yield without overflow.
- **Interactive Visual Editor:** 
  - Double-click any photo inside the grid to open a massive graphical overlay editor.
  - Dynamically drag the mathematical bounds of a photo (Crop 4-Corner Handles).
  - Interact smoothly with a responsive, rotating Audio/Volume Dial to manually rotate the source geometry.
- **Fluid Drag & Drop Integration:** Load up to 100 images purely by dragging them from your Windows system folders gracefully onto the application container.
- **Physical Workspace Saving:** Save all mathematical cuts, custom rotation parameters, margins, gaps, and custom layout variables natively back to your disk using encrypted `.ppw` configuration scripts.
- **Custom Modern Aesthetics:** Integrated GDI+ aesthetic engines force Windows 11 Modern interface styling internally—including flat panels, dynamic glowing linear gradients, and responsive hovering controls.

## Compilation 

This project requires strictly no heavy Visual Studio installations out-of-the-box if using normal GNU Compilers.

### Method 1: Windows Native Command Prompt (MinGW GCC)
1. Ensure `g++` is installed via MSYS2 / MinGW-w64 and is accessible on your system `$PATH`.
2. Open your terminal in the application project root.
3. Execute `.\build_gcc.bat`
4. Run `.\PhotoPrinter.exe`

### Method 2: Microsoft Visual C++ Compiler
1. Execute this project under `x64 Native Tools Command Prompt for VS`.
2. Run `.\build_msvc.bat`
3. Run `.\PhotoPrinter.exe`

### Method 3: Cross-Compilation on Ubuntu Linux
Deploy the Windows `.exe` heavily from within Linux natively!
1. Ensure the Windows Cross-Compiler toolkit is installed `sudo apt-get install mingw-w64`
2. Run `./build_ubuntu.sh`
3. Retrieve the generated `.exe` for use dynamically inside a Microsoft Windows host.

## Development Stack
- Language: Native C++ 11 Standard
- Interface Matrix: Windows API (Win32), Common Controls (`comctl32`), Window GUI Event Loops.
- Image Engineering: Windows GDI+ (`gdiplus.lib`), Native Memory Context BitBlts `SelectObject()`.

---

**Developed & Engineered Exclusively by Maragung**
