// =============================================================================
// textureExample - テクスチャフィルター・ラップモードのデモ
// =============================================================================

#include "tcApp.h"

void tcApp::setup() {
    cout << "=== textureExample ===" << endl;
    cout << "Texture Filter & Wrap Mode Demo" << endl;
    cout << "[UP/DOWN] Change scale" << endl;
    cout << "[1] Scale 4x" << endl;
    cout << "[2] Scale 8x" << endl;
    cout << "[3] Scale 16x" << endl;
    cout << "[4] Scale 32x" << endl;

    // 画像を生成
    imgNearest_.allocate(16, 16, 4);
    imgLinear_.allocate(16, 16, 4);

    generatePixelArt(imgNearest_);
    generatePixelArt(imgLinear_);

    // フィルター設定
    imgNearest_.setFilter(tc::TextureFilter::Nearest);
    imgLinear_.setFilter(tc::TextureFilter::Linear);

    // テクスチャ更新
    imgNearest_.update();
    imgLinear_.update();
}

void tcApp::update() {
    // 特に何もしない
}

void tcApp::draw() {
    tc::clear(0.2f, 0.2f, 0.25f);

    float w = tc::getWindowWidth();
    float h = tc::getWindowHeight();

    // タイトル
    tc::setColor(1.0f, 1.0f, 1.0f);
    tc::drawBitmapString("Texture Filter Comparison", 20, 30);

    tc::setColor(0.7f, 0.7f, 0.7f);
    tc::drawBitmapString("Left: NEAREST (pixel-perfect)  |  Right: LINEAR (smooth)", 20, 50);
    tc::drawBitmapString("Scale: " + to_string((int)scale_) + "x  [UP/DOWN or 1-4 to change]", 20, 70);

    // 画像のサイズ
    float imgW = 16 * scale_;
    float imgH = 16 * scale_;

    // 中央配置用
    float centerY = h / 2 - imgH / 2;
    float leftX = w / 4 - imgW / 2;
    float rightX = w * 3 / 4 - imgW / 2;

    // 背景（チェッカーボードで透明度を見せる）
    tc::setColor(0.3f, 0.3f, 0.35f);
    tc::drawRect(leftX - 10, centerY - 10, imgW + 20, imgH + 20);
    tc::drawRect(rightX - 10, centerY - 10, imgW + 20, imgH + 20);

    // NEAREST
    tc::setColor(1.0f, 1.0f, 1.0f);
    imgNearest_.draw(leftX, centerY, imgW, imgH);

    // LINEAR
    imgLinear_.draw(rightX, centerY, imgW, imgH);

    // ラベル
    tc::setColor(0.4f, 0.8f, 1.0f);
    tc::drawBitmapString("NEAREST", leftX + imgW / 2 - 28, centerY + imgH + 25);
    tc::setColor(1.0f, 0.8f, 0.4f);
    tc::drawBitmapString("LINEAR", rightX + imgW / 2 - 24, centerY + imgH + 25);

    // 原寸表示
    tc::setColor(1.0f, 1.0f, 1.0f);
    tc::drawBitmapString("Original (16x16):", 20, h - 80);
    imgNearest_.draw(20, h - 60, 16, 16);

    // 説明
    tc::setColor(0.6f, 0.6f, 0.6f);
    tc::drawBitmapString("NEAREST: Sharp pixels, ideal for pixel art", 20, h - 35);
    tc::drawBitmapString("LINEAR: Smooth interpolation, good for photos", 20, h - 20);

    // スケール変更のガイド
    tc::setColor(0.5f, 0.5f, 0.5f);
    float guideY = centerY + imgH + 60;
    tc::drawBitmapString("Notice how NEAREST preserves sharp edges while LINEAR blurs them", w / 2 - 200, guideY);
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

void tcApp::generatePixelArt(tc::Image& img) {
    // 16x16 のかわいいキャラクター風パターン
    // スライムっぽいやつ

    // 背景を透明に
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
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

    // スライムの形（手描き風）
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
    img.setColor(5, 7, eyeHighlight);  // ハイライト

    // 目（右）
    img.setColor(9, 7, eye);
    img.setColor(10, 7, eye);
    img.setColor(9, 8, eye);
    img.setColor(10, 8, eye);
    img.setColor(9, 7, eyeHighlight);  // ハイライト

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
