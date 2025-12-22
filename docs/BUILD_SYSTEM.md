# TrussC Build System

TrussC uses CMake with custom macros that automate most configuration. Users typically only need to edit `addons.make`.

---

## Quick Start

A minimal TrussC project needs only this CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.20)

set(TRUSSC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../trussc")
include(${TRUSSC_DIR}/cmake/trussc_app.cmake)

trussc_app()
```

That's it. Everything else is automatic.

---

## What `trussc_app()` Does

The `trussc_app()` macro handles all standard configuration:

| Task | Automatic Behavior |
|------|-------------------|
| **Project name** | Derived from folder name |
| **Source files** | Recursively collects `src/*.cpp`, `*.c`, `*.h`, `*.mm` |
| **C++ standard** | C++20 |
| **TrussC linking** | Links `tc::TrussC` |
| **Addons** | Reads `addons.make` and applies all listed addons |
| **Output directory** | `bin/` in project folder |
| **macOS bundle** | Creates `.app` bundle with proper Info.plist |
| **Windows** | Sets startup project in Visual Studio |
| **App icon** | Auto-converts PNG in `icon/` folder |

### Optional Parameters

```cmake
# Explicit project name
trussc_app(NAME myCustomName)

# Explicit source files (skip auto-collection)
trussc_app(SOURCES main.cpp app.cpp)
```

---

## Addon System

### Using Addons

Create `addons.make` in your project folder:

```
# Physics
tcxBox2d

# Networking
tcxOsc
```

Lines starting with `#` are comments. Empty lines are ignored.

### How Addon Loading Works

When `apply_addons()` is called (automatically by `trussc_app()`):

1. Reads `addons.make` line by line
2. For each addon name, calls `use_addon(target, addonName)`
3. `use_addon()` resolves the addon path: `${TC_ROOT}/addons/${addonName}`

### Addon Resolution

Each addon is processed based on whether it has a CMakeLists.txt:

**With CMakeLists.txt:**
```
tcxBox2d/
├── CMakeLists.txt  ← Used directly (full control)
├── src/
└── libs/
```
The addon's CMakeLists.txt is included via `add_subdirectory()`. Use this for:
- FetchContent dependencies
- Complex build logic
- Platform-specific configuration

**Without CMakeLists.txt:**
```
tcxSimple/
├── src/           ← Auto-collected
│   ├── tcxSimple.h
│   └── tcxSimple.cpp
└── libs/          ← Auto-collected
    └── somelib/
        ├── src/
        └── include/
```
The build system automatically:
- Collects all `.cpp`, `.c`, `.mm`, `.m` from `src/` and `libs/*/src/`
- Sets up include paths for `src/`, `include/`, and `libs/*/include/`
- Creates a static library and links TrussC

### Addon Dependencies

Addons can depend on other addons using `use_addon()` in their CMakeLists.txt:

```cmake
# tcxMyAddon/CMakeLists.txt
add_library(tcxMyAddon STATIC ...)
use_addon(tcxMyAddon tcxBox2d)  # Depends on tcxBox2d
target_link_libraries(tcxMyAddon PUBLIC TrussC)
```

Users only need to add `tcxMyAddon` to their `addons.make`; `tcxBox2d` is automatically included.

---

## Cross-Platform Builds

### macOS

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Or use Xcode:
```bash
cmake -G Xcode ..
open *.xcodeproj
```

**Output:** `bin/ProjectName.app` (macOS bundle)

### Windows

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Or open in Visual Studio:
```bash
cmake -G "Visual Studio 17 2022" ..
```

**Output:** `bin/ProjectName.exe`

### Linux

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

**Output:** `bin/ProjectName`

### Web (Emscripten)

Requires [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html).

```bash
source /path/to/emsdk/emsdk_env.sh
mkdir build-web && cd build-web
emcmake cmake ..
cmake --build .
```

**Output:** `bin/ProjectName.html` (plus `.js` and `.wasm`)

The Project Generator can create a `build-web.command` (macOS) or `build-web.bat` (Windows) script that handles this automatically.

---

## Output Structure

All builds output to the `bin/` folder in your project:

```
myProject/
├── bin/
│   ├── myProject.app/    # macOS bundle
│   │   └── Contents/
│   │       └── Resources/
│   │           └── data/  # Symlink to bin/data
│   ├── myProject.exe      # Windows
│   ├── myProject          # Linux
│   ├── myProject.html     # Web
│   └── data/              # Assets folder
└── src/
```

### Data Folder

Place assets (images, fonts, sounds) in `bin/data/`. This path is automatically resolved at runtime via `tc::getDataPath()`.

---

## App Icon

Place a 512x512+ PNG in the `icon/` folder:

```
myProject/
├── icon/
│   └── myicon.png
└── src/
```

At build time:
- **macOS:** Converted to `.icns` (requires `sips` and `iconutil`, included with Xcode)
- **Windows:** Converted to `.ico` (requires [ImageMagick](https://imagemagick.org/))

If conversion tools are not available, the icon step is skipped silently.

---

## Build Types

CMake supports multiple build types:

| Type | Description |
|------|-------------|
| `Debug` | No optimization, full debug symbols |
| `Release` | Full optimization, no debug symbols |
| `RelWithDebInfo` | Optimization with debug symbols (default) |
| `MinSizeRel` | Optimize for size |

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

For multi-config generators (Xcode, Visual Studio):
```bash
cmake --build . --config Release
```

---

## Troubleshooting

### "TrussC addon not found"

Check that:
1. Addon name in `addons.make` matches the folder name exactly
2. Addon exists in `${TC_ROOT}/addons/`

### "Cannot find TrussC.h"

Check that `TRUSSC_DIR` in CMakeLists.txt points to the correct path.

### Build is slow

- Use `cmake --build . -j` for parallel builds
- On Windows, avoid Debug builds for daily development (use RelWithDebInfo)

### Emscripten errors

Make sure to source `emsdk_env.sh` before running `emcmake cmake`.
