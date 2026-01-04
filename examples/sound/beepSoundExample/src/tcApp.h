#pragma once

#include <TrussC.h>
using namespace std;
using namespace tc;

class tcApp : public App {
public:
    void setup() override;
    void draw() override;
    void keyPressed(int key) override;
    void mousePressed(Vec2 pos, int button) override;
};
