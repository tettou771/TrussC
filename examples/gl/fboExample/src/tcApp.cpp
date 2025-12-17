#include "tcApp.h"

void tcApp::setup() {
    cout << "fboExample: FBO Demo" << endl;
    cout << "  - Press SPACE to toggle clear() in FBO" << endl;
    cout << "  - Current: using Fbo::begin(color)" << endl;

    // FBO を作成（400x300）
    fbo_.allocate(400, 300);
}

void tcApp::update() {
    time_ = tc::getElapsedTime();

    // 自動テスト: 2秒後と4秒後にスクリーンショット
    if (!test1Done_ && time_ > 2.0f) {
        fbo_.save("fbo_mode1_begin_color.png");
        cout << "Saved: fbo_mode1_begin_color.png" << endl;
        test1Done_ = true;
        useClearInFbo_ = true;  // モード切替
        cout << "Switched to tc::clear() mode" << endl;
    }
    if (!test2Done_ && time_ > 4.0f) {
        fbo_.save("fbo_mode2_clear_in_fbo.png");
        cout << "Saved: fbo_mode2_clear_in_fbo.png" << endl;
        test2Done_ = true;
        cout << "Test complete! Check the PNG files." << endl;
    }
}

void tcApp::draw() {
    // 画面をクリア
    tc::clear(30, 30, 40);

    // --- FBO にオフスクリーン描画 ---
    if (useClearInFbo_) {
        // clear() を FBO 内で呼ぶテスト
        fbo_.begin();
        tc::clear(0.2f, 0.1f, 0.3f);  // 紫色でクリア
    } else {
        // 通常: begin() の引数でクリア色を指定
        fbo_.begin(0.2f, 0.1f, 0.3f, 1.0f);
    }

    // FBO 内に描画
    // 回転する円
    int numCircles = 8;
    float centerX = fbo_.getWidth() / 2.0f;
    float centerY = fbo_.getHeight() / 2.0f;
    float radius = 100.0f;

    for (int i = 0; i < numCircles; i++) {
        float angle = (float(i) / numCircles) * tc::TAU + time_;
        float x = centerX + cos(angle) * radius;
        float y = centerY + sin(angle) * radius;

        float hue = float(i) / numCircles * tc::TAU;
        tc::Color c = tc::colorFromHSB(hue, 0.8f, 1.0f);
        tc::setColor(c);
        tc::drawCircle(x, y, 25.0f);
    }

    // 中央に白い円
    tc::setColor(255);
    tc::drawCircle(centerX, centerY, 40.0f + sin(time_ * 3.0f) * 10.0f);

    fbo_.end();

    // --- FBO を画面に描画 ---
    tc::setColor(255);

    // 左上: 等倍
    fbo_.draw(20, 80);

    // 右: 縮小
    fbo_.draw(450, 80, 200, 150);

    // --- 情報表示 ---
    string modeStr = useClearInFbo_ ? "Using tc::clear() in FBO" : "Using Fbo::begin(color)";
    tc::drawBitmapStringHighlight("FBO Example", 10, 20,
        tc::Color(0, 0, 0, 0.7f), tc::Color(1, 1, 1));
    tc::drawBitmapStringHighlight(modeStr, 10, 40,
        tc::Color(0, 0, 0, 0.7f), useClearInFbo_ ? tc::Color(1, 0.5f, 0.5f) : tc::Color(0.5f, 1, 0.5f));
    tc::drawBitmapStringHighlight("Press SPACE to toggle", 10, 60,
        tc::Color(0, 0, 0, 0.7f), tc::Color(1, 1, 1));
}

void tcApp::keyPressed(int key) {
    if (key == ' ') {
        useClearInFbo_ = !useClearInFbo_;
        if (useClearInFbo_) {
            cout << "Mode: Using tc::clear() in FBO (may not work correctly)" << endl;
        } else {
            cout << "Mode: Using Fbo::begin(color) (correct method)" << endl;
        }
    }
}
