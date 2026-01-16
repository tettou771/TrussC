#pragma once

#include <TrussC.h>
#include <tcxQuadWarp.h>
using namespace tc;
using namespace tcx;

class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    
    void keyPressed(int key) override;

private:
    QuadWarp warper_;
    Fbo testFbo;
};
