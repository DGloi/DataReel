# DataReel GTK

Desktop application for downloading media using yt-dlp with GTK4 UI.

## Prerequisites

### Linux (Debian/Ubuntu)
```bash
sudo apt-get install build-essential cmake pkg-config libgtk-4-dev yt-dlp
```

### Linux (Fedora)
```bash
sudo dnf install gcc gcc-c++ cmake pkgconfig gtk4-devel yt-dlp
```

### macOS
```bash
brew install cmake gtk4 yt-dlp
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./youtube-dl-gtk
```

## Features (Current & Planned)

- [x] Basic GTK4 UI
- [x] URL input and download path selection
- [x] Process management for downloads
- [ ] Download progress tracking
- [ ] Queue management
- [ ] Multiple concurrent downloads
- [ ] Download history
- [ ] Config file support
- [ ] Plugin architecture for other downloaders

## Architecture

The application is designed with extensibility in mind:

- **UI Layer** (`src/ui/`): GTK4 interface components
- **Core Layer** (`src/core/`): Download logic, process management
- **Utils** (`src/utils/`): Configuration, helpers

Future data acquisition tools can be integrated by implementing the downloader interface.

---

## .gitignore

```
# Build directories
build/
cmake-build-*/

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile
*.cmake
!CMakeLists.txt

# Compiled files
*.o
*.so
*.dylib
*.a
youtube-dl-gtk

# IDE
.vscode/
.idea/
*.swp
*.swo
*~

# macOS
.DS_Store

# Debug
*.dSYM/
