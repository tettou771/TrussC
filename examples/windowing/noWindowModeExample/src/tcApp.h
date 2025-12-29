#pragma once

#include <TrussC.h>
using namespace std;
using namespace tc;

class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void cleanup() override;
    void exit() override;

private:
    int counter_ = 0;
    int maxFrames_ = 300;  // Run for 5 seconds at 60fps
};
