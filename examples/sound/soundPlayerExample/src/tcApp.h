#pragma once

#include "tcBaseApp.h"
#include "tc/sound/tcSound.h"

class tcApp : public tc::App {
public:
    void setup() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    tc::Sound music;
    tc::Sound sfx;

    std::string musicPath;
    bool musicLoaded = false;
    bool sfxLoaded = false;
};
