#include "tcApp.h"

void tcApp::setup() {
    tcLogNotice("noWindowMode") << "=== noWindowMode Example ===";
    tcLogNotice("noWindowMode") << "Running without window for " << maxFrames_ << " frames (~5 seconds)";
    tcLogNotice("noWindowMode") << "Press Ctrl+C to exit early";
}

void tcApp::update() {
    counter_++;

    // Print progress every second (60 frames)
    if (counter_ % 60 == 0) {
        double elapsed = headless::getElapsedTime();
        tcLogNotice("noWindowMode") << "Frame " << counter_ << " | Elapsed: " << elapsed << "s";
    }

    // Test sokol-based methods (should not crash in headless mode)
    if (counter_ == 1) {
        tcLogNotice("noWindowMode") << "Testing sokol-based methods...";

        // These call sokol functions internally
        clear(colors::black);
        setColor(colors::red);
        drawCircle(100, 100, 50);

        tcLogNotice("noWindowMode") << "sokol methods called (may crash or be no-op)";
    }

    // Exit after maxFrames
    if (counter_ >= maxFrames_) {
        tcLogNotice("noWindowMode") << "Reached " << maxFrames_ << " frames, exiting...";
        requestExit();
    }
}

void tcApp::draw() {
    // This should not be called in headless mode
    tcLogWarning("noWindowMode") << "draw() called - this should not happen in headless mode!";
}

void tcApp::exit() {
    tcLogNotice("noWindowMode") << "exit() called";
}

void tcApp::cleanup() {
    tcLogNotice("noWindowMode") << "cleanup() called";
    tcLogNotice("noWindowMode") << "Total frames: " << counter_;
    tcLogNotice("noWindowMode") << "=== Done ===";
}
