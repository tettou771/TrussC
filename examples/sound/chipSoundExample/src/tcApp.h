#pragma once

#include <TrussC.h>
#include <vector>
#include <string>

using namespace std;
using namespace tc;

struct SoundButton {
    string label;
    Sound sound;
    float x, y, w, h;
    bool isLoop = false;      // For melody toggle
    float playEndTime = 0;    // When single-shot sound ends
};

class tcApp : public App {
public:
    void setup() override;
    void draw() override;
    void mousePressed(Vec2 pos, int button) override;

private:
    void createSounds();
    void drawButton(const SoundButton& btn, bool highlight);

    vector<SoundButton> simpleButtons;   // Section 1: Simple notes
    vector<SoundButton> chordButtons;    // Section 2: Chords
    vector<SoundButton> effectButtons;   // Section 3: Effects
    vector<SoundButton> melodyButtons;   // Section 4: Melodies

    float buttonWidth = 110;
    float buttonHeight = 40;
    float margin = 8;
};
