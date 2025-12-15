#pragma once

#include "TrussC.h"
#include "tcBaseApp.h"

class tcApp : public tc::App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    // FBO（オフスクリーンレンダリング用）
    tc::Fbo fbo;

    // スクリーンショット保存先
    std::filesystem::path savePath;
    int screenshotCount = 0;

    // 画面描画内容（デモ用）
    float time = 0.0f;
};
