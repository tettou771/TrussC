#pragma once

#include "tcBaseApp.h"
#include <iostream>

using namespace std;

// =============================================================================
// textureExample - テクスチャフィルター・ラップモードのデモ
// =============================================================================
class tcApp : public tc::App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    // テスト用画像（同じ内容で異なるフィルター設定）
    tc::Image imgNearest_;
    tc::Image imgLinear_;

    // ピクセルアート風パターンを生成
    void generatePixelArt(tc::Image& img);

    // 現在の表示スケール
    float scale_ = 16.0f;
};
