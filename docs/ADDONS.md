# TrussC アドオン

TrussC はアドオンシステムで機能を拡張できる。openFrameworks の ofxAddon に相当する仕組み。

---

## 目次

1. [設計思想](#設計思想)
2. [アドオンの使い方](#アドオンの使い方)
3. [既存アドオンの導入](#既存アドオンの導入)
4. [アドオンの作り方](#アドオンの作り方)
5. [命名規則](#命名規則)
6. [依存関係](#依存関係)

---

## 設計思想

TrussC のプロジェクト構成は **「CMakeLists.txt は全プロジェクト共通、ユーザーが編集するのは addons.make だけ」** という思想に基づいている。

### なぜこの設計か

- **シンプル**: プロジェクト固有の設定ファイルは `addons.make` のみ
- **メンテナンス容易**: CMakeLists.txt を更新する必要がない
- **間違いにくい**: 「何を編集すべきか」が明確

### ユーザーが編集するファイル

| ファイル | 用途 | 編集タイミング |
|---------|------|--------------|
| `addons.make` | 使用するアドオンの指定 | アドオンを追加/削除するとき |
| `src/*.cpp` | アプリケーションコード | 常に |

### ユーザーが編集しないファイル

| ファイル | 理由 |
|---------|------|
| `CMakeLists.txt` | 全プロジェクト共通。`trussc_app()` が全て自動処理 |

---

## アドオンの使い方

### 1. addons.make を作成

プロジェクトフォルダに `addons.make` ファイルを作成し、使用するアドオン名を1行ずつ記述:

```
tcxOsc
tcxBox2d
```

コメントも使える:

```
# 物理エンジン
tcxBox2d

# ネットワーク（後で追加予定）
# tcxOsc
```

### 2. コードで使用

```cpp
#include <tcxBox2d.h>

using namespace tcx::box2d;

World world;
Circle circle;

void setup() {
    world.setup(Vec2(0, 300));
    circle.setup(world, 400, 100, 30);
}
```

### CMakeLists.txt（参考）

CMakeLists.txt は全プロジェクト共通。編集不要:

```cmake
cmake_minimum_required(VERSION 3.20)

set(TRUSSC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../trussc")
include(${TRUSSC_DIR}/cmake/trussc_app.cmake)

trussc_app()
```

`trussc_app()` が以下を自動で行う:

- プロジェクト名をフォルダ名から取得
- `src/*.cpp` を自動収集してビルド
- TrussC をリンク
- `addons.make` からアドオンを自動追加
- macOS バンドル設定、アイコン設定など

---

## 既存アドオンの導入

### 公式アドオン

TrussC に同梱されているアドオン:

| アドオン名 | 説明 |
|-----------|------|
| tcxBox2d | Box2D 2D物理エンジン |
| tcxOsc | OSC プロトコル送受信 |
| tcxTls | TLS/SSL 通信（mbedTLS） |

### サードパーティアドオン

1. アドオンを `addons/` フォルダに配置
2. `addons.make` に追加

```
tc_v0.0.1/
└── addons/
    ├── tcxBox2d/        # 公式
    ├── tcxOsc/          # 公式
    └── tcxSomeAddon/    # サードパーティ
```

---

## アドオンの作り方

### フォルダ構造

```
addons/tcxMyAddon/
├── src/                     # アドオンのコード (.h + .cpp 一緒)
│   ├── tcxMyAddon.h         # メインヘッダー（全部 include）
│   ├── tcxMyClass.h
│   └── tcxMyClass.cpp
├── libs/                    # 外部ライブラリ（git submodule等）
│   └── somelib/
├── example-basic/           # サンプル（srcと同階層、example-xxx形式）
│   ├── src/
│   │   ├── main.cpp
│   │   └── tcApp.cpp
│   ├── addons.make          # このサンプルで使うアドオン
│   └── CMakeLists.txt       # 共通テンプレート
└── CMakeLists.txt           # オプション（FetchContent等が必要な場合のみ）
```

**ポイント:**
- `src/`: アドオン自体のコード。`.h` と `.cpp` を同じ場所に置く
- `libs/`: 外部から持ってきたソースコード、git submodule など
- `example-xxx/`: サンプルは `src/` と同階層に配置。CMakeLists.txt は共通テンプレート
- `CMakeLists.txt`: 基本的に不要。FetchContent など特殊処理が必要な場合のみ作成

### CMakeLists.txt が不要なケース

以下の条件を満たすアドオンは CMakeLists.txt 不要:

- `src/` 内のソースのみで完結
- 外部ライブラリ不要、または `libs/` 内にソースコードとして含まれている
- TrussC 以外への依存なし

### CMakeLists.txt が必要なケース

FetchContent で外部ライブラリを取得する場合:

```cmake
# tcxBox2d/CMakeLists.txt

set(ADDON_NAME tcxBox2d)

# Box2D を FetchContent で取得
include(FetchContent)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v2.4.1
)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(box2d)

# ソースファイル
file(GLOB ADDON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB ADDON_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

add_library(${ADDON_NAME} STATIC ${ADDON_SOURCES} ${ADDON_HEADERS})
target_include_directories(${ADDON_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${ADDON_NAME} PUBLIC box2d TrussC)
```

### メインヘッダー（tcxMyAddon.h）

```cpp
// =============================================================================
// tcxMyAddon - アドオン説明
// =============================================================================

#pragma once

// 全てのヘッダーをインクルード
#include "tcxMyClass.h"
#include "tcxAnotherClass.h"
```

### クラス実装例

```cpp
// tcxMyClass.h
#pragma once

#include <TrussC.h>

namespace tcx::myaddon {

class MyClass {
public:
    void setup();
    void update();
    void draw();

private:
    // ...
};

} // namespace tcx::myaddon
```

---

## 命名規則

### アドオン名

| 項目 | 規則 | 例 |
|-----|------|-----|
| フォルダ名 | `tcx` + PascalCase | `tcxBox2d`, `tcxGui` |
| ライブラリ名 | フォルダ名と同じ | `tcxBox2d` |
| メインヘッダー | `アドオン名.h` | `tcxBox2d.h` |

### 名前空間

```cpp
namespace tcx::アドオン名 {
    // ...
}
```

| アドオン | 名前空間 |
|---------|---------|
| tcxBox2d | `tcx::box2d` |
| tcxGui | `tcx::gui` |
| tcxOsc | `tcx::osc` |

**注意:** TrussC 本体は `tc::` を使用。アドオンは `tcx::` を使用。

### ファイル名

| 種類 | 規則 | 例 |
|-----|------|-----|
| ヘッダー | `tcx` + PascalCase + `.h` | `tcxBox2dWorld.h` |
| 実装 | `tcx` + PascalCase + `.cpp` | `tcxBox2dWorld.cpp` |

### クラス名

名前空間内ではプレフィックスなし:

```cpp
namespace tcx::box2d {
    class World { ... };    // OK: tcx::box2d::World
    class Circle { ... };   // OK: tcx::box2d::Circle
}
```

---

## 依存関係

### 他のアドオンに依存する場合

アドオンの CMakeLists.txt 内で `use_addon()` を使用:

```cmake
# tcxMyAddon が tcxBox2d に依存
add_library(${ADDON_NAME} STATIC ...)

use_addon(${ADDON_NAME} tcxBox2d)

target_link_libraries(${ADDON_NAME} PUBLIC TrussC)
```

ユーザーが `addons.make` に `tcxMyAddon` を書くだけで、tcxBox2d も自動的に追加される。

### 外部ライブラリに依存する場合

FetchContent を使用:

```cmake
include(FetchContent)

FetchContent_Declare(
    somelib
    GIT_REPOSITORY https://github.com/example/somelib.git
    GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(somelib)

target_link_libraries(${ADDON_NAME} PUBLIC somelib)
```

---

## 公式アドオン一覧

### tcxBox2d

Box2D 2D物理エンジンの TrussC ラッパー。

**機能:**
- World（物理ワールド）
- Body（物理ボディ基底クラス、tc::Node 継承）
- Circle, Rect, Polygon（形状）
- マウスドラッグ（b2MouseJoint）

**使用例:**

```cpp
#include <tcxBox2d.h>

using namespace tc;
using namespace tcx::box2d;

class tcApp : public App {
    World world;
    Circle circle;

    void setup() override {
        world.setup(Vec2(0, 300));  // 重力
        world.createBounds();        // 画面端に壁
        circle.setup(world, 400, 100, 30);
    }

    void update() override {
        world.update();
        circle.updateTree();  // Box2D → Node 座標同期
    }

    void draw() override {
        clear(30);
        setColor(255, 200, 100);
        circle.drawTree();    // 位置・回転を適用して描画
    }
};
```

### tcxOsc

OSC（Open Sound Control）プロトコルの送受信。

**機能:**
- OscSender（OSC メッセージ送信）
- OscReceiver（OSC メッセージ受信）
- OscMessage, OscBundle（メッセージ構築）

### tcxTls

TLS/SSL 通信サポート（mbedTLS 使用）。

**機能:**
- セキュアな TCP 通信
- 証明書検証
