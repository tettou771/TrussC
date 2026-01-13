#pragma once

#include <TrussC.h>
using namespace std;
using namespace tc;

// =============================================================================
// ColorBox - Simple colored box for layout demo
// =============================================================================
class ColorBox : public RectNode {
public:
    using Ptr = shared_ptr<ColorBox>;

    Color boxColor = Color(0.3f, 0.3f, 0.4f);
    string label;

    ColorBox(float w = 100, float h = 40, Color color = Color(0.3f, 0.3f, 0.4f))
        : boxColor(color) {
        setSize(w, h);
        enableEvents();
    }

    void draw() override {
        // Background
        setColor(isMouseOver() ? boxColor * 1.2f : boxColor);
        fill();
        drawRect(0, 0, getWidth(), getHeight());

        // Border
        noFill();
        setColor(0.6f, 0.6f, 0.7f);
        drawRect(0, 0, getWidth(), getHeight());

        // Label
        if (!label.empty()) {
            setColor(1.0f, 1.0f, 1.0f);
            float textX = getWidth() / 2 - label.length() * 4;
            float textY = getHeight() / 2 + 4;
            drawBitmapString(label, textX, textY);
        }
    }
};

// =============================================================================
// LayoutContainer - Container with LayoutMod for demo
// =============================================================================
class LayoutContainer : public RectNode {
public:
    using Ptr = shared_ptr<LayoutContainer>;

    Color bgColor = Color(0.15f, 0.15f, 0.2f);
    string title;
    LayoutMod* layout = nullptr;

    LayoutContainer(float w, float h) {
        setSize(w, h);
    }

    void draw() override {
        // Background
        setColor(bgColor);
        fill();
        drawRect(0, 0, getWidth(), getHeight());

        // Border
        noFill();
        setColor(0.4f, 0.4f, 0.5f);
        drawRect(0, 0, getWidth(), getHeight());

        // Title (above container)
        if (!title.empty()) {
            setColor(0.8f, 0.8f, 0.85f);
            drawBitmapString(title, 5, -15);
        }
    }
};

// =============================================================================
// Main app
// =============================================================================
class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    LayoutContainer::Ptr vContainer_;
    LayoutContainer::Ptr hContainer_;
    LayoutContainer::Ptr gridContainer_;

    int boxCounter_ = 0;

    void addBoxToVStack();
    void addBoxToHStack();
    void removeLastFromVStack();
    void removeLastFromHStack();
};
