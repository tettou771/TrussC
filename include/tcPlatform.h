#pragma once

#include <string>

// =============================================================================
// プラットフォーム固有機能
// =============================================================================

namespace trussc {
namespace platform {

// メインディスプレイのDPIスケールを取得（ウィンドウ作成前でも使用可能）
// macOS: 1.0 (通常) or 2.0 (Retina)
// その他: 1.0
float getDisplayScaleFactor();

// ウィンドウサイズを変更（論理サイズで指定）
// macOS: NSWindow を使用
void setWindowSize(int width, int height);

// 実行ファイルの絶対パスを取得
std::string getExecutablePath();

// 実行ファイルがあるディレクトリを取得（末尾に / 付き）
std::string getExecutableDir();

} // namespace platform
} // namespace trussc
