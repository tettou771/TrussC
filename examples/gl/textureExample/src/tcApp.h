#pragma once

#include "tcBaseApp.h"
#include <iostream>

using namespace std;

// =============================================================================
// textureExample - テクスチャフィルター比較デモ（Nearest / Linear / Cubic）
// =============================================================================
class tcApp : public tc::App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    // 元画像（16x16）
    tc::Image imgOriginal_;

    // 表示用画像（異なるフィルター設定）
    tc::Image imgNearest_;
    tc::Image imgLinear_;
    tc::Image imgCubic_;  // CPU でバイキュービック補間済み

    // ピクセルアート風パターンを生成
    void generatePixelArt(tc::Image& img);

    // バイキュービック補間でアップスケール
    void upscaleBicubic(const tc::Image& src, tc::Image& dst, int newWidth, int newHeight);

    // バイキュービック補間の重み関数（Catmull-Rom スプライン）
    float cubicWeight(float t);

    // 現在の表示スケール
    float scale_ = 16.0f;
    float lastScale_ = 0.0f;  // 前回のスケール（変化検知用）

    // 元画像サイズ
    static constexpr int SRC_SIZE = 16;
};
