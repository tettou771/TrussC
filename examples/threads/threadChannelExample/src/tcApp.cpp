#include "tcApp.h"
#include <cmath>

// ---------------------------------------------------------------------------
// setup - 初期化
// ---------------------------------------------------------------------------
void tcApp::setup() {
    sourcePixels_.resize(AnalysisThread::TOTAL_PIXELS);
}

// ---------------------------------------------------------------------------
// update - 更新
// ---------------------------------------------------------------------------
void tcApp::update() {
    frameNum_++;

    // パターンを生成（sin波 + ノイズ）
    float t = frameNum_ * 0.05f;
    for (int i = 0; i < AnalysisThread::TOTAL_PIXELS; i++) {
        float ux = (i % AnalysisThread::WIDTH) / (float)AnalysisThread::WIDTH;
        float uy = (i / AnalysisThread::WIDTH) / (float)AnalysisThread::HEIGHT;

        // 波模様パターン
        float value = std::sin(ux * 10.0f + t) * std::sin(uy * 8.0f + t * 0.7f);
        value = (value + 1.0f) * 0.5f;  // 0-1 に正規化
        sourcePixels_[i] = value;
    }

    // 解析リクエストを送信
    analyzer_.analyze(sourcePixels_);

    // 解析結果を受信
    analyzer_.update();
}

// ---------------------------------------------------------------------------
// draw - 描画
// ---------------------------------------------------------------------------
void tcApp::draw() {
    tc::clear(0.1f, 0.1f, 0.1f);

    // 元のパターンを描画（左側）
    tc::setColor(255);
    tc::drawBitmapString("Source (Main Thread)", 20, 20);

    for (int py = 0; py < AnalysisThread::HEIGHT; py++) {
        for (int px = 0; px < AnalysisThread::WIDTH; px++) {
            int i = py * AnalysisThread::WIDTH + px;
            float value = sourcePixels_[i];
            tc::setColor(value, value, value);
            tc::drawRect(20 + px * 4, 40 + py * 4, 4, 4);
        }
    }

    // 解析結果を描画（右側）
    tc::setColor(255);
    tc::drawBitmapString("Analyzed (Worker Thread)", 300, 20);
    analyzer_.draw(300, 40);

    // 情報表示
    tc::setColor(200, 200, 200);
    tc::drawBitmapString("Frame: " + tc::toString(frameNum_), 20, 260);
    tc::drawBitmapString("Analyzed: " + tc::toString(analyzer_.getAnalyzedCount()), 20, 275);

    tc::setColor(100, 200, 100);
    tc::drawBitmapString("ThreadChannel Demo:", 20, 310);
    tc::drawBitmapString("  Main -> Worker: source pixels", 20, 325);
    tc::drawBitmapString("  Worker -> Main: thresholded result", 20, 340);
}
