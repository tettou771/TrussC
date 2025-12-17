// =============================================================================
// textureExample - テクスチャフィルター比較デモ（Nearest / Linear / Cubic）
// =============================================================================

#include "tcApp.h"
#include <cmath>
#include <algorithm>

void tcApp::setup() {
    cout << "=== textureExample ===" << endl;
    cout << "Texture Filter Comparison: Nearest / Linear / Cubic" << endl;
    cout << "[UP/DOWN] Change scale" << endl;
    cout << "[1] Scale 4x" << endl;
    cout << "[2] Scale 8x" << endl;
    cout << "[3] Scale 16x" << endl;
    cout << "[4] Scale 32x" << endl;

    // 元画像を生成
    imgOriginal_.allocate(SRC_SIZE, SRC_SIZE, 4);
    generatePixelArt(imgOriginal_);
    imgOriginal_.update();

    // Nearest 用（元画像をコピー）
    imgNearest_.allocate(SRC_SIZE, SRC_SIZE, 4);
    generatePixelArt(imgNearest_);
    imgNearest_.setFilter(tc::TextureFilter::Nearest);
    imgNearest_.update();

    // Linear 用（元画像をコピー）
    imgLinear_.allocate(SRC_SIZE, SRC_SIZE, 4);
    generatePixelArt(imgLinear_);
    imgLinear_.setFilter(tc::TextureFilter::Linear);
    imgLinear_.update();

    // Cubic 用は update() で生成（スケール変更時に再生成）
}

void tcApp::update() {
    // スケールが変わったら Cubic 画像を再生成
    if (scale_ != lastScale_) {
        int newSize = (int)(SRC_SIZE * scale_);
        upscaleBicubic(imgOriginal_, imgCubic_, newSize, newSize);
        imgCubic_.setFilter(tc::TextureFilter::Nearest);  // 補間済みなので Nearest で表示
        imgCubic_.update();
        lastScale_ = scale_;
    }
}

void tcApp::draw() {
    tc::clear(0.2f, 0.2f, 0.25f);

    float w = tc::getWindowWidth();
    float h = tc::getWindowHeight();

    // タイトル
    tc::setColor(1.0f, 1.0f, 1.0f);
    tc::drawBitmapString("Texture Filter Comparison", 20, 30);

    tc::setColor(0.7f, 0.7f, 0.7f);
    tc::drawBitmapString("NEAREST (sharp) | LINEAR (GPU blur) | CUBIC (CPU bicubic)", 20, 50);
    tc::drawBitmapString("Scale: " + to_string((int)scale_) + "x  [UP/DOWN or 1-4 to change]", 20, 70);

    // 画像のサイズ
    float imgW = SRC_SIZE * scale_;
    float imgH = SRC_SIZE * scale_;

    // 3列配置
    float margin = 20;
    float availWidth = w - margin * 4;
    float colWidth = availWidth / 3;
    float centerY = h / 2 - imgH / 2;

    float x1 = margin + (colWidth - imgW) / 2;
    float x2 = margin * 2 + colWidth + (colWidth - imgW) / 2;
    float x3 = margin * 3 + colWidth * 2 + (colWidth - imgW) / 2;

    // 背景
    tc::setColor(0.3f, 0.3f, 0.35f);
    tc::drawRect(x1 - 5, centerY - 5, imgW + 10, imgH + 10);
    tc::drawRect(x2 - 5, centerY - 5, imgW + 10, imgH + 10);
    tc::drawRect(x3 - 5, centerY - 5, imgW + 10, imgH + 10);

    // 画像描画
    tc::setColor(1.0f, 1.0f, 1.0f);

    // NEAREST（GPU スケール）
    imgNearest_.draw(x1, centerY, imgW, imgH);

    // LINEAR（GPU スケール）
    imgLinear_.draw(x2, centerY, imgW, imgH);

    // CUBIC（CPU プリスケール済み、等倍表示）
    imgCubic_.draw(x3, centerY, imgW, imgH);

    // ラベル
    tc::setColor(0.4f, 0.8f, 1.0f);
    tc::drawBitmapString("NEAREST", x1 + imgW / 2 - 28, centerY + imgH + 20);
    tc::setColor(1.0f, 0.8f, 0.4f);
    tc::drawBitmapString("LINEAR", x2 + imgW / 2 - 24, centerY + imgH + 20);
    tc::setColor(0.8f, 1.0f, 0.4f);
    tc::drawBitmapString("CUBIC", x3 + imgW / 2 - 20, centerY + imgH + 20);

    // 原寸表示
    tc::setColor(1.0f, 1.0f, 1.0f);
    tc::drawBitmapString("Original (16x16):", 20, h - 80);
    imgOriginal_.draw(20, h - 60, SRC_SIZE, SRC_SIZE);

    // 説明
    tc::setColor(0.6f, 0.6f, 0.6f);
    tc::drawBitmapString("NEAREST: Sharp pixels, ideal for pixel art", 20, h - 35);
    tc::drawBitmapString("LINEAR: GPU bilinear, smooth but blurry | CUBIC: CPU bicubic, smoother curves", 20, h - 20);
}

void tcApp::keyPressed(int key) {
    if (key == SAPP_KEYCODE_UP) {
        scale_ = min(scale_ * 2.0f, 64.0f);
        cout << "Scale: " << scale_ << "x" << endl;
    } else if (key == SAPP_KEYCODE_DOWN) {
        scale_ = max(scale_ / 2.0f, 1.0f);
        cout << "Scale: " << scale_ << "x" << endl;
    } else if (key == '1') {
        scale_ = 4.0f;
    } else if (key == '2') {
        scale_ = 8.0f;
    } else if (key == '3') {
        scale_ = 16.0f;
    } else if (key == '4') {
        scale_ = 32.0f;
    }
}

// ---------------------------------------------------------------------------
// バイキュービック補間の重み関数（Catmull-Rom スプライン）
// ---------------------------------------------------------------------------
float tcApp::cubicWeight(float t) {
    t = fabs(t);
    if (t < 1.0f) {
        return (1.5f * t - 2.5f) * t * t + 1.0f;
    } else if (t < 2.0f) {
        return ((-0.5f * t + 2.5f) * t - 4.0f) * t + 2.0f;
    }
    return 0.0f;
}

// ---------------------------------------------------------------------------
// バイキュービック補間でアップスケール
// ---------------------------------------------------------------------------
void tcApp::upscaleBicubic(const tc::Image& src, tc::Image& dst, int newWidth, int newHeight) {
    int srcW = src.getWidth();
    int srcH = src.getHeight();

    dst.allocate(newWidth, newHeight, 4);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            // 元画像での対応位置（小数）
            float srcX = (x + 0.5f) * srcW / newWidth - 0.5f;
            float srcY = (y + 0.5f) * srcH / newHeight - 0.5f;

            // 整数部と小数部
            int ix = (int)floor(srcX);
            int iy = (int)floor(srcY);
            float fx = srcX - ix;
            float fy = srcY - iy;

            // 4x4 ピクセルから補間
            float r = 0, g = 0, b = 0, a = 0;
            float weightSum = 0;

            for (int dy = -1; dy <= 2; dy++) {
                float wy = cubicWeight(fy - dy);
                int sy = clamp(iy + dy, 0, srcH - 1);

                for (int dx = -1; dx <= 2; dx++) {
                    float wx = cubicWeight(fx - dx);
                    int sx = clamp(ix + dx, 0, srcW - 1);

                    float w = wx * wy;
                    tc::Color c = src.getColor(sx, sy);

                    r += c.r * w;
                    g += c.g * w;
                    b += c.b * w;
                    a += c.a * w;
                    weightSum += w;
                }
            }

            // 正規化してクランプ
            if (weightSum > 0) {
                r /= weightSum;
                g /= weightSum;
                b /= weightSum;
                a /= weightSum;
            }

            r = clamp(r, 0.0f, 1.0f);
            g = clamp(g, 0.0f, 1.0f);
            b = clamp(b, 0.0f, 1.0f);
            a = clamp(a, 0.0f, 1.0f);

            dst.setColor(x, y, tc::Color(r, g, b, a));
        }
    }
}

// ---------------------------------------------------------------------------
// ピクセルアート生成（スライム）
// ---------------------------------------------------------------------------
void tcApp::generatePixelArt(tc::Image& img) {
    // 背景を透明に
    for (int y = 0; y < SRC_SIZE; y++) {
        for (int x = 0; x < SRC_SIZE; x++) {
            img.setColor(x, y, tc::Color(0, 0, 0, 0));
        }
    }

    // 本体の色（緑のスライム）
    tc::Color body(0.3f, 0.8f, 0.4f, 1.0f);
    tc::Color bodyLight(0.5f, 0.9f, 0.6f, 1.0f);
    tc::Color bodyDark(0.2f, 0.6f, 0.3f, 1.0f);
    tc::Color eye(0.1f, 0.1f, 0.1f, 1.0f);
    tc::Color eyeHighlight(1.0f, 1.0f, 1.0f, 1.0f);
    tc::Color mouth(0.15f, 0.15f, 0.15f, 1.0f);

    // スライムの形
    // Row 4-5: 頭の上部
    for (int x = 5; x <= 10; x++) { img.setColor(x, 4, bodyLight); }
    for (int x = 4; x <= 11; x++) { img.setColor(x, 5, body); }

    // Row 6-9: 顔部分
    for (int x = 3; x <= 12; x++) { img.setColor(x, 6, body); }
    for (int x = 3; x <= 12; x++) { img.setColor(x, 7, body); }
    for (int x = 3; x <= 12; x++) { img.setColor(x, 8, body); }
    for (int x = 3; x <= 12; x++) { img.setColor(x, 9, body); }

    // Row 10-12: 体下部
    for (int x = 4; x <= 11; x++) { img.setColor(x, 10, body); }
    for (int x = 5; x <= 10; x++) { img.setColor(x, 11, bodyDark); }
    for (int x = 6; x <= 9; x++) { img.setColor(x, 12, bodyDark); }

    // ハイライト（左上）
    img.setColor(5, 5, bodyLight);
    img.setColor(6, 5, bodyLight);
    img.setColor(4, 6, bodyLight);
    img.setColor(5, 6, bodyLight);

    // 目（左）
    img.setColor(5, 7, eye);
    img.setColor(6, 7, eye);
    img.setColor(5, 8, eye);
    img.setColor(6, 8, eye);
    img.setColor(5, 7, eyeHighlight);

    // 目（右）
    img.setColor(9, 7, eye);
    img.setColor(10, 7, eye);
    img.setColor(9, 8, eye);
    img.setColor(10, 8, eye);
    img.setColor(9, 7, eyeHighlight);

    // 口（にっこり）
    img.setColor(7, 9, mouth);
    img.setColor(8, 9, mouth);

    // 輪郭（暗い色）
    img.setColor(3, 7, bodyDark);
    img.setColor(3, 8, bodyDark);
    img.setColor(3, 9, bodyDark);
    img.setColor(12, 7, bodyDark);
    img.setColor(12, 8, bodyDark);
    img.setColor(12, 9, bodyDark);
}
