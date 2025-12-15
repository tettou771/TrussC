#include "tcApp.h"
#include <iostream>

using namespace std;

void tcApp::setup() {
    cout << "windowExample: Loop Architecture Demo" << endl;
    cout << "  - 1: VSync (default)" << endl;
    cout << "  - 2: Fixed 30 FPS" << endl;
    cout << "  - 3: Fixed 5 FPS" << endl;
    cout << "  - 4: Event-driven (click to redraw)" << endl;
    cout << "  - 5: Decoupled Update (500Hz) + VSync Draw" << endl;
    cout << "  - ESC: Quit" << endl;

    // デフォルト: VSync
    tc::setVsync(true);
}

void tcApp::update() {
    updateCount++;
}

void tcApp::draw() {
    drawCount++;

    // 1秒ごとにカウンタリセット
    float t = tc::getElapsedTime();
    if (t - lastResetTime >= 1.0f) {
        lastResetTime = t;
        updateCount = 0;
        drawCount = 0;
    }

    // 背景色をモードで変える
    switch (mode) {
        case 0: tc::clear(0.1f, 0.1f, 0.2f); break;  // VSync: 青
        case 1: tc::clear(0.1f, 0.2f, 0.1f); break;  // 30fps: 緑
        case 2: tc::clear(0.2f, 0.2f, 0.1f); break;  // 5fps: 黄
        case 3: tc::clear(0.2f, 0.1f, 0.1f); break;  // Event: 赤
        case 4: tc::clear(0.2f, 0.1f, 0.2f); break;  // Decoupled: 紫
        default: tc::clear(0.1f, 0.1f, 0.1f); break;
    }

    // 回転する四角形（アニメーション確認用）
    float angle = tc::getElapsedTime();
    tc::pushMatrix();
    tc::translate(tc::getWindowWidth() / 2, tc::getWindowHeight() / 2);
    tc::rotate(angle);
    tc::setColor(1.0f, 1.0f, 1.0f);
    tc::drawRect(-250, -50, 500, 100);
    tc::popMatrix();

    // 情報表示
    tc::setColor(1.0f, 1.0f, 1.0f);
    float y = 20;

    tc::drawBitmapString("Loop Architecture Demo", 10, y); y += 20;
    y += 10;

    // 現在のモード
    string modeStr;
    switch (mode) {
        case 0: modeStr = "VSync (default)"; break;
        case 1: modeStr = "Fixed 30 FPS"; break;
        case 2: modeStr = "Fixed 5 FPS"; break;
        case 3: modeStr = "Event-driven"; break;
        case 4: modeStr = "Decoupled (Update 500Hz)"; break;
    }
    tc::drawBitmapString("Mode: " + modeStr, 10, y); y += 16;
    y += 10;

    // 設定状態
    tc::drawBitmapString("Draw VSync: " + string(tc::isDrawVsync() ? "ON" : "OFF"), 10, y); y += 16;
    tc::drawBitmapString("Draw FPS setting: " + tc::toString(tc::getDrawFps()), 10, y); y += 16;
    tc::drawBitmapString("Update synced: " + string(tc::isUpdateSyncedToDraw() ? "YES" : "NO"), 10, y); y += 16;
    tc::drawBitmapString("Update FPS setting: " + tc::toString(tc::getUpdateFps()), 10, y); y += 16;
    y += 10;

    // 実際のFPS
    tc::drawBitmapString("Actual FPS: " + tc::toString(tc::getFrameRate(), 1), 10, y); y += 16;
    tc::drawBitmapString("Update/sec: " + tc::toString(updateCount), 10, y); y += 16;
    tc::drawBitmapString("Draw/sec: " + tc::toString(drawCount), 10, y); y += 16;
    y += 20;

    // 操作説明
    tc::drawBitmapString("Press 1-5 to change mode", 10, y); y += 16;
    if (mode == 3) {
        tc::drawBitmapString("Click to redraw!", 10, y); y += 16;
    }
}

void tcApp::keyPressed(int key) {
    if (key == tc::KEY_ESCAPE) {
        sapp_request_quit();
    }
    else if (key == '1') {
        mode = 0;
        tc::setVsync(true);
        cout << "Mode: VSync" << endl;
    }
    else if (key == '2') {
        mode = 1;
        tc::setFps(30);
        cout << "Mode: Fixed 30 FPS" << endl;
    }
    else if (key == '3') {
        mode = 2;
        tc::setFps(5);
        cout << "Mode: Fixed 5 FPS" << endl;
    }
    else if (key == '4') {
        mode = 3;
        tc::setDrawFps(0);  // 自動描画停止
        tc::syncUpdateToDraw(true);
        tc::redraw();  // モード切り替え直後に1回描画
        cout << "Mode: Event-driven (click to redraw)" << endl;
    }
    else if (key == '5') {
        mode = 4;
        tc::setDrawVsync(true);
        tc::setUpdateFps(500);  // Update は 500Hz で独立
        cout << "Mode: Decoupled (Update 500Hz, Draw VSync)" << endl;
    }
}

void tcApp::mousePressed(int x, int y, int button) {
    // イベント駆動モードでは、クリックで再描画
    if (mode == 3) {
        tc::redraw();
    }
}
