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
    Mesh core = createSphere(0.35f, 16);
    spaceStation.append(core);

    // Habitat ring (torus)
    Mesh ring = createTorus(1.2f, 0.15f, 16, 24);
    spaceStation.append(ring);

    // Connection spokes (4 cylinders from core to ring, 2x thicker than solar panel arms)
    for (int i = 0; i < 4; i++) {
        Mesh spoke = createCylinder(0.08f, 0.85f, 8);  // 0.08 = 0.04 * 2
        spoke.rotateX(HALF_TAU / 2);
        spoke.translate(0, 0, 0.6f);  // Center between core and ring
        spoke.rotateY(i * TAU / 4);   // Rotate around Y
        spaceStation.append(spoke);
    }

    // Solar panels (flat boxes on each end)
    for (int side = -1; side <= 1; side += 2) {
        // Panel arm (spans from core to panels)
        Mesh arm = createCylinder(0.04f, 1.4f, 6);
        arm.translate(0, side * 0.7f, 0);  // Center the arm between core and panels
        spaceStation.append(arm);

        // Solar panel (flat box)
        Mesh panel = createBox(1.5f, 0.02f, 0.4f);
        panel.translate(0, side * 1.4f, 0);
        spaceStation.append(panel);

        // Second panel
        Mesh panel2 = createBox(1.5f, 0.02f, 0.4f);
        panel2.translate(0, side * 1.4f, 0.5f);
        spaceStation.append(panel2);

        Mesh panel3 = createBox(1.5f, 0.02f, 0.4f);
        panel3.translate(0, side * 1.4f, -0.5f);
        spaceStation.append(panel3);
    }

    // Docking modules (small cylinders at ends of core)
    for (int side = -1; side <= 1; side += 2) {
        Mesh dock = createCylinder(0.2f, 0.3f, 8);
        dock.rotateX(HALF_TAU / 2);
        dock.translate(side * 1.2f, 0, 0);
        spaceStation.append(dock);

        // Docking cone
        Mesh cone = createCone(0.12f, 0.15f, 8);
        cone.rotateZ(side * HALF_TAU / 2);
        cone.translate(side * 1.4f, 0, 0);
        spaceStation.append(cone);
    }
}

void tcApp::update() {
    rotationY += 0.005f;
    rotationX = sin(getElapsedTimef() * 0.3f) * 0.2f;
}

void tcApp::draw() {
    clear(0.05f);

    // Setup 3D view
    enable3DPerspective(deg2rad(50.0f), 0.1f, 100.0f);

    pushMatrix();
    translate(0, 0, -5);
    rotateX(rotationX);
    rotateY(rotationY);

    // Draw the space station
    setColor(0.7f, 0.75f, 0.8f);
    spaceStation.draw();

    // Draw wireframe overlay
    setColor(0.3f, 0.4f, 0.5f);
    spaceStation.drawWireframe();

    popMatrix();

    // Return to 2D mode
    disable3D();
}
