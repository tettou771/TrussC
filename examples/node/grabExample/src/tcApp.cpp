// =============================================================================
// grabExample - Mouse grab/drag test
// =============================================================================

#include "tcApp.h"

void tcApp::setup() {
    // Draggable rect
    draggable_ = make_shared<DraggableRect>(120, 80);
    draggable_->setPos(50, 100);
    addChild(draggable_);

    // Drawing canvas with clipping
    canvas_ = make_shared<DrawingCanvas>(400, 350);
    canvas_->setPos(200, 80);
    addChild(canvas_);

    logNotice("tcApp") << "=== grabExample ===";
    logNotice("tcApp") << "Drag the rect to move it";
    logNotice("tcApp") << "Draw on the canvas (try dragging outside!)";
    logNotice("tcApp") << "Press C to clear canvas";
}

void tcApp::draw() {
    clear(0.08f, 0.08f, 0.1f);

    // Title
    setColor(0.8f, 0.8f, 0.85f);
    drawBitmapString("Grab/Drag Test", 50, 30);

    // Instructions
    setColor(0.5f, 0.5f, 0.55f);
    drawBitmapString("Press C to clear canvas", 50, 55);
}

void tcApp::keyPressed(int key) {
    if (key == 'c' || key == 'C') {
        canvas_->clear();
    }
}
