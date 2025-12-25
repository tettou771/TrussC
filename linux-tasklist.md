# Linux Support Tasklist

## Development Environment Setup

### Required Packages (Ubuntu/Debian)

```bash
sudo apt install -y \
    libx11-dev \
    libxcursor-dev \
    libxi-dev \
    libxrandr-dev \
    libgl1-mesa-dev \
    libasound2-dev \
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libavutil-dev \
    pkg-config
```

| Package | Purpose |
|---------|---------|
| `libx11-dev` | X11 window system |
| `libxcursor-dev` | Cursor management |
| `libxi-dev` | X Input Extension |
| `libxrandr-dev` | Display settings (resolution, etc.) |
| `libgl1-mesa-dev` | OpenGL |
| `libasound2-dev` | ALSA (audio) |
| `libgtk-3-dev` | GTK3 (file dialogs) |
| `libavcodec-dev` | FFmpeg video/audio codecs |
| `libavformat-dev` | FFmpeg container formats |
| `libswscale-dev` | FFmpeg pixel format conversion |
| `libavutil-dev` | FFmpeg utilities |
| `pkg-config` | Library discovery and configuration |

### TODO
- [ ] Create setup script (`setup_linux.sh`) to automate package installation

---

## Build Status

### emptyExample
- [x] CMake configure
- [x] Build
- [x] Run - Window displayed successfully!

**Build Notes:**
- Fixed `CHAR_WIDTH` macro conflict with Linux `limits.h` (renamed to `GLYPH_WIDTH`)
- Created `tcPlatform_linux.cpp` with basic implementations

### All Examples (build_all.sh)

**Summary: 41/41 Success! 🎉 Full Linux support achieved**

| Category | Example | Build | Run | Notes |
|----------|---------|-------|-----|-------|
| 3d | 3DPrimitivesExample | OK | OK | |
| 3d | easyCamExample | OK | OK | |
| 3d | ofNodeExample | OK | OK | |
| communication | serialExample | OK | OK | Tested with Arduino (needs dialout group) |
| events | eventsExample | OK | OK | |
| events | hitTestExample | OK | OK | |
| events | uiExample | OK | OK | |
| graphics | blendingExample | OK | OK | |
| graphics | clippingExample | OK | OK | |
| graphics | colorExample | OK | OK | |
| graphics | fontExample | OK | OK | Uses DejaVuSans (default on Ubuntu) |
| graphics | graphicsExample | OK | OK | |
| graphics | polylinesExample | OK | OK | |
| graphics | strokeMeshExample | OK | OK | |
| gui | imguiExample | OK | OK | |
| input_output | dragDropExample | OK | OK | |
| input_output | fileDialogExample | OK | OK | GTK3 dialogs work |
| input_output | imageLoaderExample | OK | OK | |
| input_output | jsonXmlExample | OK | OK | |
| input_output | keyboardExample | OK | OK | |
| input_output | mouseExample | OK | OK | |
| input_output | screenshotExample | OK | OK | PNG saved correctly |
| math | noiseField2dExample | OK | OK | |
| math | vectorMathExample | OK | OK | |
| network | tcpExample | OK | BUG | Freeze after disconnect (see Issues) |
| network | udpExample | OK | OK | |
| sound | micInputExample | OK | - | No mic to test |
| sound | soundPlayerExample | OK | OK | Verified with CAVA |
| sound | soundPlayerFFTExample | OK | OK | |
| templates | emptyExample | OK | OK | |
| threads | threadChannelExample | OK | OK | |
| threads | threadExample | OK | OK | |
| tools | projectGenerator | OK | OK | Create + build works |
| utils | clipboardExample | OK | OK | |
| utils | consoleExample | OK | OK | Tested via pipe |
| utils | timerExample | OK | OK | |
| utils | utilsExample | OK | OK | |
| video | videoGrabberExample | OK | OK | V4L2 implementation (needs video group) |
| video | videoPlayerExample | OK | OK | FFmpeg-based implementation |
| video | videoPlayerWebExample | OK | - | Uses same FFmpeg implementation |
| windowing | loopModeExample | OK | OK | |

---

## Issues Found

### tcpExample - Freeze after disconnect
- **Symptom**: After client disconnect, the app stops responding to keyboard input (e.g., pressing S to restart server)
- **Reproduction**: Start server -> Connect client -> Disconnect -> Try to start server again
- **Platform**: Linux only (needs verification on macOS/Windows)
- **Priority**: Medium

---

## Platform Implementation Status

| Feature | File | Status | Notes |
|---------|------|--------|-------|
| getDisplayScaleFactor | `tcPlatform_linux.cpp` | | |
| setWindowSize | `tcPlatform_linux.cpp` | | |
| getExecutablePath | `tcPlatform_linux.cpp` | | |
| captureWindow | `tcPlatform_linux.cpp` | | |
| saveScreenshot | `tcPlatform_linux.cpp` | | |
| FBO pixel reading | `tcFbo_linux.cpp` | | |
| VideoGrabber | `tcVideoGrabber_linux.cpp` | | |
