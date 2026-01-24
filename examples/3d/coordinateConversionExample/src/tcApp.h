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

private:
    EasyCam cam;
    Mesh boxMesh;
    Mesh sphereMesh;

    // World positions of objects
    Vec3 boxPos{100, 0, 0};
    Vec3 spherePos{-100, 0, 0};

    // Click handling
    Vec2 clickScreenPos{0, 0};
    Vec3 clickWorldPos{0, 0, 0};
    bool pendingClick = false;
    bool hasClick = false;
};
