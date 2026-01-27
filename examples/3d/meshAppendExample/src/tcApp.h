#pragma once

#include <TrussC.h>
using namespace std;
using namespace tc;

class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;

private:
    Mesh spaceStation;
    float rotationY = 0;
    float rotationX = 0;
};
