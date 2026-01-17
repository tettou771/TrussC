# tcxTcv - TrussC Video Codec

TrussC向け独自ビデオコーデック。UI録画やモーショングラフィックスなど静止領域が多い映像に最適化。

## Features

- **GPU-native**: BC7テクスチャ圧縮でGPU直接デコード（超高速再生）
- **高速シーク**: I/P/REFフレーム構造で任意位置へ即座にシーク可能
- **効率的圧縮**: SKIP + BC7 + LZ4 による高圧縮率
- **アルファ対応**: RGBA形式で透過動画もOK
- **音声対応**: AAC/MP3/PCM音声埋め込み＆同期再生
- **並列デコード**: Chunked LZ4でマルチスレッド展開（v5）

## File Format (v5)

```
.tcv file structure:

Header (64 bytes):
  - signature: "TCVC"
  - version: 5
  - width, height, frameCount, fps
  - blockSize: 16 (fixed)
  - audioCodec, audioSampleRate, audioChannels
  - audioOffset, audioSize

Frame packets:
  - I_FRAME: [type][chunkCount][uncompSize][chunkSizes...][LZ4 chunks...]
  - P_FRAME: [type][refFrame][uncompSize][compSize][LZ4 data]
  - REF_FRAME: [type][refFrame]

Audio chunk (at end of file):
  - Raw audio data (AAC/MP3/PCM)
```

## Usage

### Player

```cpp
#include <tcxTcv.h>

TcvPlayer player;
player.load("video.tcv");
player.play();

void update() {
    player.update();
}

void draw() {
    player.draw(0, 0);
}
```

### Encoder

```cpp
#include <tcxTcv.h>

TcvEncoder encoder;
encoder.setQuality(1);      // 0=fast, 1=balanced, 2=high
encoder.setMaxChunks(16);   // LZ4 chunks for parallel decode (1-16)
encoder.begin("output.tcv", width, height, fps);

for (int i = 0; i < frameCount; i++) {
    encoder.addFrame(pixelData);  // RGBA
}

encoder.end();
```

### Encoder App

```bash
# GUI mode (drag & drop)
./TrussC_Video_Codec_Encoder

# CLI mode
./TrussC_Video_Codec_Encoder -i input.mov -o output.tcv -q balanced --chunks 16
```

## Compression Details

| Block Type | Description | Size |
|------------|-------------|------|
| SKIP | Same as reference frame | 0 bytes |
| BC7 | 16x16 block (16 BC7 blocks) | 256 bytes |

- RLE: 1-128連続ブロックを1バイトで表現
- LZ4: フレームデータをチャンク分割＆並列圧縮/展開

## Performance (1920x1080)

| Metric | v4 | v5 |
|--------|----|----|
| I-frame decode | ~1.6ms | ~1.1ms |
| P-frame decode | ~0.5ms | ~0.5ms |
| File size | ~50-70% of raw BC7 | same |

## Audio Support

| Codec | Embed | Playback (macOS) | Playback (Windows) |
|-------|-------|------------------|-------------------|
| AAC | Yes | Yes | Yes |
| MP3 | Yes | Yes | Yes |
| PCM | Yes | Yes | Yes |

## Examples

- `TrussC_Video_Codec_Encoder` - エンコーダアプリ（GUI/CLI両対応）
- `example-tcvPlayer` - 再生サンプル

## Dependencies

- LZ4 (bundled in libs/)
- bc7enc (header-only, MIT)
- bcdec (header-only, MIT)

## License

MIT License
