# TrussC ビルド方法

## 前提条件

- CMake 3.20 以上
- C++20 対応コンパイラ
- macOS: Xcode Command Line Tools (`xcode-select --install`)
- Windows: Visual Studio 2022 または MinGW

---

## 環境変数の設定

TrussC を使うには `TRUSSC_PATH` 環境変数を設定する。

### フォルダ構造

```
/path/to/TrussC/           ← TRUSSC_PATH はここを指す
├── trussc_v0.0.1/         ← バージョンごとのフォルダ
├── trussc_v0.1.0/
└── ...
```

### macOS / Linux

```bash
# ~/.zshrc または ~/.bashrc に追加
export TRUSSC_PATH="/path/to/TrussC"
```

設定を反映：
```bash
source ~/.zshrc
```

### Windows

```powershell
# システム環境変数に追加
setx TRUSSC_PATH "C:\path\to\TrussC"
```

または「システムのプロパティ」→「環境変数」から GUI で設定。

---

## 新規プロジェクトの作成

### 1. テンプレートをコピー

```bash
cp -r /path/to/TrussC/trussc_v0.0.1/examples/templates/emptyExample ~/myProject
cd ~/myProject
```

### 2. バージョンを確認・変更（任意）

`CMakeLists.txt` の先頭付近：

```cmake
set(TRUSSC_VERSION "0.0.1" CACHE STRING "TrussC version to use")
```

使いたいバージョンに変更可能。

### 3. ビルド

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 4. 実行

```bash
# macOS
open bin/myProject.app

# Windows
./bin/myProject.exe

# Linux
./bin/myProject
```

**注意:** 出力先は `bin/` フォルダ（openFrameworks と同じスタイル）。

---

## ビルドオプション

### リリースビルド

```bash
cmake --build . --config Release
```

### クリーンビルド

```bash
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

### バージョンを cmake コマンドで指定

```bash
cmake .. -DTRUSSC_VERSION=0.1.0
```

---

## アドオンの追加

`CMakeLists.txt` に1行追加するだけ：

```cmake
# TrussC をリンク
target_link_libraries(${PROJECT_NAME} PRIVATE tc::TrussC)

# アドオンを追加
use_addon(${PROJECT_NAME} tcxBox2d)
```

詳しくは [ADDONS.md](ADDONS.md) を参照。

---

## IDE での開発

### VSCode

1. 拡張機能をインストール：
   ```bash
   code --install-extension ms-vscode.cmake-tools
   code --install-extension ms-vscode.cpptools
   ```

2. プロジェクトフォルダを開く：
   ```bash
   code ~/myProject
   ```

3. CMake Tools が自動で検出

4. ビルド: `F7` または `Cmd+Shift+P` → `CMake: Build`

5. 実行: `Cmd+Shift+P` → `CMake: Run Without Debugging`

### Xcode（macOS）

```bash
cd ~/myProject
mkdir build && cd build
cmake -G Xcode ..
open *.xcodeproj
```

Xcode 内で `Cmd+R` で実行。

### Visual Studio（Windows）

```bash
cd myProject
mkdir build && cd build
cmake -G "Visual Studio 17 2022" ..
start *.sln
```

Visual Studio 内で `F5` で実行。

### CLion

1. CLion でプロジェクトフォルダを開く
2. 自動で CMake を認識
3. 右上の再生ボタンでビルド & 実行

---

## TrussC 同梱サンプルのビルド

TrussC に同梱されているサンプルをビルドする場合（開発者向け）：

```bash
cd /path/to/TrussC/trussc_v0.0.1/examples/graphics/graphicsExample
mkdir build && cd build
cmake ..
cmake --build .
open bin/graphicsExample.app
```

---

## トラブルシューティング

### TRUSSC_PATH が設定されていない

```
╔══════════════════════════════════════════════════════════════════╗
║   ERROR: TRUSSC_PATH environment variable is not set!           ║
╚══════════════════════════════════════════════════════════════════╝
```

→ 環境変数を設定して、ターミナルを再起動。

### 指定したバージョンが見つからない

```
╔══════════════════════════════════════════════════════════════════╗
║   ERROR: TrussC v0.0.1 not found!                               ║
║   Available versions in /path/to/TrussC:                        ║
║     - trussc_v0.0.1                                             ║
╚══════════════════════════════════════════════════════════════════╝
```

→ CMakeLists.txt の `TRUSSC_VERSION` を利用可能なバージョンに変更。

### CMake が見つからない

```bash
# macOS (Homebrew)
brew install cmake

# Windows (winget)
winget install Kitware.CMake
```

### コンパイラが見つからない（macOS）

```bash
xcode-select --install
```

### ビルドエラーが出る

```bash
# build フォルダを削除してやり直し
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```
