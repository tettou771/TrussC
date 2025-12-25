# TrussC Roadmap

Development roadmap based on feature comparison with openFrameworks.

---

## Current Status

**Platforms:** macOS, Windows, Linux, Web (Emscripten)

### Implemented Features

| Category | Features |
|----------|----------|
| **Graphics** | Shapes, Image, Fonts (Bitmap/TrueType), Path, Mesh, StrokeMesh, Clipping, Blend modes |
| **3D** | Transforms, Primitives, EasyCam, Node, RectNode, Lighting, Materials |
| **GPU** | Shader, FBO, Texture |
| **Math** | Vec2/3/4, Mat3/4, Perlin Noise, Ray, Color spaces (RGB/HSB/OKLab/OKLCH) |
| **Events** | Keyboard, Mouse, Window resize, Drag & drop, Event<T>, RectNode events |
| **Sound** | SoundPlayer, SoundStream (mic input), ChipSound |
| **Video** | VideoPlayer (FFmpeg), VideoGrabber (webcam) |
| **Network** | TCP Client/Server, UDP |
| **Utils** | Timer, Thread, Serial, File dialogs, JSON/XML, Clipboard |
| **UI** | Dear ImGui integration |
| **Addons** | tcxTls (TLS/SSL), tcxOsc (OSC), tcxBox2d (physics) |

---

## Unimplemented Features

### Priority: Medium

| Feature | Description | Difficulty |
|---------|-------------|------------|
| 3D model loading | OBJ/glTF | High |
| Texture mapping | UV mapping to Mesh | Medium |
| Normal mapping | Bump mapping | High |
| Spot light | Spotlight support | Medium |

### Priority: Low

| Feature | Description | Difficulty |
|---------|-------------|------------|
| VBO detail control | Dynamic vertex buffers | Medium |
| Particle system | Consider as addon | Medium |
| Touch input | iOS/Android | High |

---

## Future Samples

| Category | Sample | Description | Priority |
|----------|--------|-------------|----------|
| 3d/ | modelLoaderExample | OBJ/glTF model loading | Medium |
| graphics/ | particleExample | Particle system | Medium |
| animation/ | tweenExample | Easing functions | Medium |
| animation/ | spriteSheetExample | Sprite animation | Low |
| game/ | pongExample | Simple game demo | Low |
| generative/ | flowFieldExample | Flow field art | Low |
| generative/ | lSystemExample | L-System generation | Low |

---

## Raspberry Pi Support (Planned)

Priority: Low | Difficulty: Medium

### TODO
- [ ] Check sokol OpenGL ES backend support
- [ ] Decide RPi version support (4/5 only or include 3)
- [ ] Add ARM architecture detection to CMake
- [ ] Test V4L2 with RPi camera
- [ ] Verify miniaudio ALSA on ARM
- [ ] Test on actual hardware

---

## External Library Updates

TrussC depends on several external libraries.
Image processing libraries are particularly prone to vulnerabilities, so **check for latest versions with each release**.

| Library | Purpose | Update Priority | Notes |
|:--------|:--------|:----------------|:------|
| **stb_image** | Image loading | **High** | Many CVEs, always use latest |
| **stb_image_write** | Image writing | **High** | Same as above |
| **stb_truetype** | Font rendering | Medium | |
| pugixml | XML parsing | Medium | |
| nlohmann/json | JSON parsing | Medium | |
| sokol | Rendering backend | Medium | **TrussC has customizations (see below)** |
| miniaudio | Audio | Medium | |
| Dear ImGui | GUI | Low | Use stable versions |

**Update Checklist:**
- Check GitHub Release Notes / Security Advisories
- For stb, check commit history at https://github.com/nothings/stb (no tags)

### sokol Customizations

`sokol_app.h` has customizations to prevent flickering during event-driven rendering.
When updating sokol, these changes must be reapplied:

1. Add `bool skip_present;` flag to `_sapp_t` struct
2. Add API declaration `SOKOL_APP_API_DECL void sapp_skip_present(void);`
3. Add skip check at top of `_sapp_d3d11_present()`
4. Add `sapp_skip_present()` implementation

Details: `git log --oneline -p -- trussc/include/sokol/sokol_app.h`

---

## Reference Links

- [oF Examples](https://github.com/openframeworks/openFrameworks/tree/master/examples)
- [oF Documentation](https://openframeworks.cc/documentation/)
- [sokol](https://github.com/floooh/sokol)
