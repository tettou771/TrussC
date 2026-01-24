// =============================================================================
// roundedRectExample
// Compares three rectangle drawing methods:
//   - drawRect: sharp corners
//   - drawRectRounded: circular arc corners
//   - drawRectSquircle: curvature-continuous corners (iOS-style)
// =============================================================================

#include "tcApp.h"

void tcApp::setup() {
    setWindowTitle("Rounded Rect Example");
    setCircleResolution(64);
}

void tcApp::draw() {
    clear(0.15f, 0.15f, 0.2f);

    float w = 200, h = 200, r = 40;
    float y = 200;

    // 1. Normal
    setColor(1.0f);
    drawBitmapString("drawRect", 100, y - 20);
    setColor(0.9f, 0.3f, 0.3f);
    drawRect(100, y, w, h);

    // 2. Rounded (circular arc)
    setColor(1.0f);
    drawBitmapString("drawRectRounded", 380, y - 20);
    setColor(0.3f, 0.9f, 0.3f);
    drawRectRounded(380, y, w, h, r);

    // 3. Squircle (iOS-style)
    setColor(1.0f);
    drawBitmapString("drawRectSquircle", 660, y - 20);
    setColor(0.3f, 0.5f, 0.9f);
    drawRectSquircle(660, y, w, h, r);
}

void tcApp::keyPressed(int key) {
    if (key == KEY_ESCAPE) sapp_request_quit();
}
