// beepSoundExample - Debug beep sound presets

#include "tcApp.h"

void tcApp::setup() {
}

void tcApp::draw() {
    clear(0.12f);

    setColor(0.7f);
    drawBitmapString(R"(dbg::beep() - Debug Sound Presets

[Basic]       1: ping
[Positive]    2: success     3: complete    4: coin
[Negative]    5: error       6: warning     7: cancel
[UI]          8: click       9: typing      0: notify
[Transition]  -: sweep

UP/DOWN: Volume    Click: ping)", 50, 50);

    // Volume bar
    float y = 200;
    setColor(0.5f);
    drawBitmapString(format("Volume: {:.0f}%", dbg::getBeepVolume() * 100), 50, y);
    setColor(0.3f);
    drawRect(170, y - 3, 150, 14);
    setColor(colors::lime);
    drawRect(170, y - 3, 150 * dbg::getBeepVolume(), 14);
}

void tcApp::keyPressed(int key) {
    switch (key) {
        case '1': dbg::beep(dbg::Beep::ping); break;
        case '2': dbg::beep(dbg::Beep::success); break;
        case '3': dbg::beep(dbg::Beep::complete); break;
        case '4': dbg::beep(dbg::Beep::coin); break;
        case '5': dbg::beep(dbg::Beep::error); break;
        case '6': dbg::beep(dbg::Beep::warning); break;
        case '7': dbg::beep(dbg::Beep::cancel); break;
        case '8': dbg::beep(dbg::Beep::click); break;
        case '9': dbg::beep(dbg::Beep::typing); break;
        case '0': dbg::beep(dbg::Beep::notify); break;
        case '-': dbg::beep(dbg::Beep::sweep); break;
        case KEY_UP:
            dbg::setBeepVolume(dbg::getBeepVolume() + 0.1f);
            dbg::beep();
            break;
        case KEY_DOWN:
            dbg::setBeepVolume(dbg::getBeepVolume() - 0.1f);
            dbg::beep();
            break;
    }
}

void tcApp::mousePressed(Vec2 pos, int button) {
    dbg::beep();
}
