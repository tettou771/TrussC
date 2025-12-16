# TrussC ロードマップ

openFrameworksとの機能比較に基づいた開発ロードマップ。

---

## 実装済み機能

### Graphics（描画）
- [x] 基本図形（rect, circle, ellipse, line, triangle, point）
- [x] Image（読み込み・描画・保存）
- [x] BitmapFont（テクスチャアトラス）
- [x] TrueTypeFont（stb_truetype ベース）
- [x] Shape API（beginShape/vertex/endShape）
- [x] Polyline（頂点配列・曲線生成）
- [x] Mesh（頂点・色・インデックス）
- [x] StrokeMesh（太線描画）
- [x] Scissor Clipping（再帰対応）

### 3D
- [x] 3D変形（translate/rotate/scale）
- [x] 深度テスト・背面カリング
- [x] 3Dプリミティブ（Plane, Box, Sphere, Cylinder, Cone, Torus, IcoSphere）
- [x] EasyCam（マウス操作3Dカメラ）
- [x] Node（シーングラフ）
- [x] RectNode（2D UI、Ray-based Hit Test）

### Math
- [x] Vec2, Vec3, Vec4
- [x] Mat3, Mat4
- [x] Perlin Noise（1D〜4D）
- [x] Ray（Hit Test用）
- [x] 色空間（RGB, HSB, OKLab, OKLCH）
- [x] ユーティリティ（lerp, clamp, map, radians, degrees）

### Events
- [x] キーボード（keyPressed, keyReleased）
- [x] マウス（pressed, released, moved, dragged, scrolled）
- [x] ウィンドウリサイズ
- [x] Event<T> テンプレート
- [x] EventListener（RAII）
- [x] RectNode イベント（mousePressed/Released/Dragged/Scrolled）

### GL
- [x] Shader（フルスクリーンシェーダー）
- [x] FBO（フレームバッファオブジェクト）

### Utils
- [x] Timer（Node::addTimerFunction）
- [x] Thread, ThreadChannel
- [x] Serial通信
- [x] フレーム制御（FPS/VSync）

---

## 未実装機能（優先度別）

### 優先度: 高

| 機能 | 説明 | 難易度 |
|------|------|--------|
| **ブレンドモード** | ADD, MULTIPLY, SCREEN 等 | 中 |
| **ライティング** | Ambient, Diffuse, Specular | 高 |
| **テクスチャ詳細制御** | NEAREST/LINEAR補間、WRAP/CLAMP | 中 |
| **ファイルダイアログ** | OS標準のファイル選択ダイアログ | 中 |
| **Log機能** | 柔軟なログシステム（詳細は下記） | 中 |

### 優先度: 中

| 機能 | 説明 | 難易度 |
|------|------|--------|
| パス描画（曲線） | ベジェ曲線、円弧 | 中 |
| 3Dモデル読み込み | obj/gltf | 高 |
| マテリアル | テクスチャ・法線マップ | 高 |
| ドラッグ&ドロップ | ファイルD&D受け取り | 低 |

### 優先度: 低

| 機能 | 説明 | 難易度 |
|------|------|--------|
| VBO詳細制御 | 動的頂点バッファ | 中 |
| パーティクルシステム | アドオン化も検討 | 中 |
| タッチ入力 | iOS/Android向け | 高 |
| ネットワーク | TCP/UDP | 高 |

### 別ワークツリーで進行中

| 機能 | 状態 |
|------|------|
| サウンド再生/入力 | 進行中 |
| VideoGrabber（Webカメラ） | 進行中 |
| ビデオ再生 | 未着手 |

---

## 機能設計メモ

### Log機能（tcLog）

oFのofLogをベースに、以下の改良を加える:

```
設計方針:
- 複数出力先を同時サポート（Console + File + カスタム）
- チャンネル（カテゴリ）ごとにログレベルを個別設定
- リスナーパターンで画面表示等に対応
```

**API案:**
```cpp
// 基本使用
tc::log(tc::LogLevel::Warning, "Something happened");
tc::logVerbose("詳細情報");
tc::logNotice("通知");
tc::logWarning("警告");
tc::logError("エラー");

// 出力先追加（複数同時可能）
tc::Log::addChannel<tc::ConsoleLogChannel>();
tc::Log::addChannel<tc::FileLogChannel>("app.log");
tc::Log::addChannel<MyCustomChannel>();  // 画面表示等

// チャンネル別ログレベル設定
tc::Log::getChannel("console")->setLevel(tc::LogLevel::Notice);
tc::Log::getChannel("file")->setLevel(tc::LogLevel::Verbose);

// カテゴリ別ログレベル
tc::Log::setLevelForModule("network", tc::LogLevel::Verbose);
tc::Log::setLevelForModule("graphics", tc::LogLevel::Warning);

// リスナー（画面表示用など）
tc::Log::addListener([](const tc::LogMessage& msg) {
    // 画面にログを表示
    myLogDisplay.add(msg.text);
});
```

**LogLevel:**
```cpp
enum class LogLevel {
    Verbose,  // 詳細デバッグ
    Notice,   // 通知
    Warning,  // 警告
    Error,    // エラー
    Fatal,    // 致命的エラー
    Silent    // 出力なし
};
```

### ブレンドモード

```cpp
enum class BlendMode {
    Alpha,      // デフォルト
    Add,        // 加算
    Multiply,   // 乗算
    Screen,     // スクリーン
    Subtract,   // 減算
    Disabled    // ブレンドなし
};

tc::setBlendMode(tc::BlendMode::Add);
```

### テクスチャ詳細制御

```cpp
// 補間モード
enum class TextureFilter {
    Nearest,    // ドット絵向け
    Linear      // デフォルト
};

// ラップモード
enum class TextureWrap {
    Repeat,
    ClampToEdge,
    MirroredRepeat
};

// 使用例
tc::Image img;
img.setMinFilter(tc::TextureFilter::Nearest);
img.setMagFilter(tc::TextureFilter::Nearest);
img.setWrap(tc::TextureWrap::Repeat);
```

---

## サンプル一覧

### 実装済み

| カテゴリ | サンプル |
|---------|---------|
| templates/ | emptyExample |
| graphics/ | graphicsExample, colorExample, clippingExample |
| 3d/ | ofNodeExample, 3DPrimitivesExample, easyCamExample |
| math/ | vectorMathExample |
| events/ | hitTestExample, uiExample |
| gl/ | screenshotExample（FBO） |

### 今後

| カテゴリ | サンプル | 優先度 |
|---------|---------|--------|
| graphics/ | fontsExample, blendingExample | 高 |
| 3d/ | lightingExample | 高 |
| input_output/ | fileDialogExample | 高 |
| gl/ | shaderExample, textureExample | 中 |
| sound/ | soundPlayerExample | 進行中 |
| video/ | videoCaptureExample | 進行中 |

---

## 参考リンク

- [oF Examples](https://github.com/openframeworks/openFrameworks/tree/master/examples)
- [oF Documentation](https://openframeworks.cc/documentation/)
- [sokol](https://github.com/floooh/sokol)
