#pragma once

// =============================================================================
// tcImage_platform.h - Image クラスのプラットフォーム固有実装
// =============================================================================

namespace trussc {

// スクリーンキャプチャ（プラットフォーム固有）
// TODO: FBO を実装してそこからピクセルを読む方式に変更
inline bool Image::grabScreenPlatform(int x, int y, int w, int h) {
    // sokol_gfx には直接フレームバッファを読む API がない
    // FBO (オフスクリーンレンダリング) を実装するまでは未対応

    // 暫定: 灰色で埋める（デバッグ用）
    if (pixels_.isAllocated()) {
        for (int py = 0; py < pixels_.getHeight(); py++) {
            for (int px = 0; px < pixels_.getWidth(); px++) {
                pixels_.setColor(px, py, Color(0.5f, 0.5f, 0.5f, 1.0f));
            }
        }
        // テクスチャを更新
        dirty_ = true;
        update();
    }

    // 未実装を示す
    return false;
}

} // namespace trussc
