#include "tcApp.h"
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

void tcApp::setup() {
    cout << "screenshotExample: FBO Save Demo (with MSAA)" << endl;
    cout << "  - Press SPACE to save FBO" << endl;
    cout << "  - FBO uses 4x MSAA for smooth edges" << endl;

    // FBO を画面サイズで確保（4x MSAA）
    fbo.allocate(tc::getWindowWidth(), tc::getWindowHeight(), 4);

    // 保存先パス（dataフォルダ）
    savePath = tc::getDataPath("");
    cout << "Screenshots will be saved to: " << savePath << endl;
}

void tcApp::update() {
    time = tc::getElapsedTime();
}

void tcApp::draw() {
    // 画面をクリア（これでスワップチェーンパスが開始される）
    tc::clear(30);

    // FBO に描画
    fbo.begin(0.2f, 0.2f, 0.3f, 1.0f);  // 暗い青紫背景

    // デモ描画：回転する円たち
    int numCircles = 12;
    float centerX = tc::getWindowWidth() / 2.0f;
    float centerY = tc::getWindowHeight() / 2.0f;
    float radius = 150.0f;

    for (int i = 0; i < numCircles; i++) {
        float angle = (float(i) / numCircles) * tc::TAU + time;
        float x = centerX + cos(angle) * radius;
        float y = centerY + sin(angle) * radius;

        // 色相を変える
        float hue = float(i) / numCircles * tc::TAU;  // 0-TAU
        tc::Color c = tc::colorFromHSB(hue, 0.8f, 1.0f);
        tc::setColor(c);

        float circleRadius = 30.0f + sin(time * 2.0f + i) * 10.0f;
        tc::drawCircle(x, y, circleRadius);
    }

    // 中央に大きな円
    tc::setColor(255);
    tc::drawCircle(centerX, centerY, 50.0f + sin(time * 3.0f) * 20.0f);

    // グリッド線
    tc::setColor(1.0f, 1.0f, 1.0f, 0.2f);
    for (int x = 0; x < tc::getWindowWidth(); x += 50) {
        tc::drawLine(x, 0, x, tc::getWindowHeight());
    }
    for (int y = 0; y < tc::getWindowHeight(); y += 50) {
        tc::drawLine(0, y, tc::getWindowWidth(), y);
    }

    fbo.end();

    // FBO の内容を画面に描画
    tc::setColor(255);
    fbo.draw(0, 0);

    // 情報表示（FBO の外でテキスト描画）
    tc::drawBitmapStringHighlight("FBO Save Demo (MSAA)", 10, 20,
        tc::Color(0, 0, 0, 0.7f), tc::Color(1, 1, 1));

    string msaaStr = "FBO: " + to_string(fbo.getWidth()) + "x" + to_string(fbo.getHeight())
                   + " (" + to_string(fbo.getSampleCount()) + "x MSAA)";
    tc::drawBitmapStringHighlight(msaaStr, 10, 40,
        tc::Color(0, 0, 0, 0.7f), tc::Color(1, 1, 1));

    tc::drawBitmapStringHighlight("Press SPACE to save", 10, 60,
        tc::Color(0, 0, 0, 0.7f), tc::Color(1, 1, 1));

    string countStr = "Saved: " + to_string(screenshotCount);
    tc::drawBitmapStringHighlight(countStr, 10, 80,
        tc::Color(0, 0, 0, 0.7f), tc::Color(1, 1, 1));
}

void tcApp::keyPressed(int key) {
    if (key == ' ') {
        // FBO の内容を直接保存
        string filename = "screenshot_" + to_string(screenshotCount) + ".png";
        fs::path filepath = savePath / filename;

        if (fbo.save(filepath)) {
            cout << "Saved: " << filepath << endl;
            screenshotCount++;
        } else {
            cout << "Failed to save: " << filepath << endl;
        }
    }
}
