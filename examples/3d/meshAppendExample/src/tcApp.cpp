// =============================================================================
// meshAppendExample
// =============================================================================
// Demonstrates Mesh::append() and transform methods (translate, rotate, scale).
//
// This example builds a 3D space station by combining multiple primitives
// (sphere, torus, cylinder, box, cone) into a single Mesh. Each primitive is
// transformed before being appended, allowing complex models to be constructed
// from simple building blocks.
//
// Key features demonstrated:
//   - Mesh::append()   : Combine multiple meshes into one
//   - Mesh::translate(): Move a mesh in 3D space
//   - Mesh::rotateX/Y/Z(): Rotate a mesh around an axis
//   - createSphere(), createTorus(), createCylinder(), createBox(), createCone()
// =============================================================================

#include "tcApp.h"

void tcApp::setup() {
    setWindowTitle("meshAppendExample");

    // Build a space station by combining primitives

    // Central core (sphere)
    Mesh core = createSphere(52.5f, 16);
    spaceStation.append(core);

    // Habitat ring (torus)
    Mesh ring = createTorus(180.0f, 22.5f, 16, 24);
    spaceStation.append(ring);

    // Connection spokes (4 cylinders from core to ring, 2x thicker than solar panel arms)
    for (int i = 0; i < 4; i++) {
        Mesh spoke = createCylinder(12.0f, 127.5f, 8);
        spoke.rotateX(HALF_TAU / 2);
        spoke.translate(0, 0, 90.0f);  // Center between core and ring
        spoke.rotateY(i * TAU / 4);    // Rotate around Y
        spaceStation.append(spoke);
    }

    // Solar panels (flat boxes on each end)
    for (int side = -1; side <= 1; side += 2) {
        // Panel arm (spans from core to panels)
        Mesh arm = createCylinder(6.0f, 210.0f, 6);
        arm.translate(0, side * 105.0f, 0);  // Center the arm between core and panels
        spaceStation.append(arm);

        // Solar panel (flat box)
        Mesh panel = createBox(225.0f, 3.0f, 60.0f);
        panel.translate(0, side * 210.0f, 0);
        spaceStation.append(panel);

        // Second panel
        Mesh panel2 = createBox(225.0f, 3.0f, 60.0f);
        panel2.translate(0, side * 210.0f, 75.0f);
        spaceStation.append(panel2);

        Mesh panel3 = createBox(225.0f, 3.0f, 60.0f);
        panel3.translate(0, side * 210.0f, -75.0f);
        spaceStation.append(panel3);
    }

    // Docking modules (small cylinders at ends of core)
    for (int side = -1; side <= 1; side += 2) {
        Mesh dock = createCylinder(30.0f, 45.0f, 8);
        dock.rotateX(HALF_TAU / 2);
        dock.translate(side * 180.0f, 0, 0);
        spaceStation.append(dock);

        // Docking cone
        Mesh cone = createCone(18.0f, 22.5f, 8);
        cone.rotateZ(side * HALF_TAU / 2);
        cone.translate(side * 210.0f, 0, 0);
        spaceStation.append(cone);
    }
}

void tcApp::update() {
    rotationY += 0.005f;
    rotationX = sin(getElapsedTimef() * 0.3f) * 0.2f;
}

void tcApp::draw() {
    clear(0.05f);

    pushMatrix();
    translate(getWidth() / 2, getHeight() / 2, 0);
    rotateX(rotationX);
    rotateY(rotationY);

    // Draw the space station
    setColor(0.7f, 0.75f, 0.8f);
    spaceStation.draw();

    // Draw wireframe overlay
    setColor(0.3f, 0.4f, 0.5f);
    spaceStation.drawWireframe();

    popMatrix();
}

void tcApp::keyPressed(int key) {
    logNotice() << "keyPressed: " << key;
    requestQuit();
}