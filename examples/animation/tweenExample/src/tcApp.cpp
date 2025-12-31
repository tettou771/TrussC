/*
 * tweenExample - Demonstrates easing functions and Tween class
 *
 * This example shows:
 * - All EaseType curves (Linear, Quad, Cubic, Quart, Quint, Sine, Expo, Circ, Back, Elastic, Bounce)
 * - EaseMode (In, Out, InOut)
 * - Tween<T> class with complete event handling
 * - Asymmetric easing (different In/Out types)
 *
 * Click anywhere to restart the animation.
 */

#include "TrussC.h"
using namespace std;
using namespace tc;

// Easing type names for display
const vector<string> easeTypeNames = {
    "Linear", "Quad", "Cubic", "Quart", "Quint",
    "Sine", "Expo", "Circ", "Back", "Elastic", "Bounce"
};

class tcApp : public App {
public:
    // Tweens for each easing type
    vector<Tween<float>> tweens;
    vector<EventListener> completeListeners;

    // Animation state
    float startX = 200;
    float endX = 900;
    float duration = 2.0f;
    EaseMode currentMode = EaseMode::InOut;
    int completedCount = 0;

    void setup() override {
        setWindowTitle("Tween Example - Click to restart");

        initTweens();
    }

    void initTweens() {
        tweens.clear();
        completeListeners.clear();
        completedCount = 0;

        // Create a tween for each easing type
        for (int i = 0; i < (int)easeTypeNames.size(); i++) {
            EaseType type = static_cast<EaseType>(i);

            Tween<float> tween;
            tween.from(startX)
                 .to(endX)
                 .duration(duration)
                 .ease(type, currentMode);

            tweens.push_back(move(tween));
        }

        // Set up complete event listeners
        completeListeners.resize(tweens.size());
        for (size_t i = 0; i < tweens.size(); i++) {
            tweens[i].complete->listen(completeListeners[i], [this, i]() {
                completedCount++;
                logNotice("Tween") << easeTypeNames[i] << " completed! ("
                    << completedCount << "/" << tweens.size() << ")";
            });
        }

        // Start all tweens
        for (auto& tween : tweens) {
            tween.start();
        }
    }

    void update() override {
        float dt = getDeltaTime();
        for (auto& tween : tweens) {
            tween.update(dt);
        }
    }

    void draw() override {
        clear(0.1f);

        float y = 70;
        float rowHeight = 60;
        float circleRadius = 12;

        // Layout constants
        float labelX = 30;
        float graphX = 110;
        float graphW = 70;
        float graphH = 40;
        float trackStartX = 200;
        float trackEndX = 900;

        // Draw header
        setColor(1.0f);
        drawBitmapString("EaseMode: " + getModeString(currentMode) + " (press 1/2/3 to change)", 30, 30);

        // Draw each easing type
        for (size_t i = 0; i < tweens.size(); i++) {
            EaseType type = static_cast<EaseType>(i);
            Color c = colorFromHSB(i * 0.09f, 0.7f, 1.0f);

            // Draw label
            setColor(0.7f);
            drawBitmapString(easeTypeNames[i], labelX, y + 5);

            // Draw individual graph for this easing type
            drawMiniGraph(graphX, y - graphH/2 + 5, graphW, graphH, type, c);

            // Draw track
            setColor(0.3f);
            drawLine(trackStartX, y, trackEndX, y);

            // Get eased position from tween
            float animX = tweens[i].getValue();

            // Draw progress bar (thin line under track)
            float progress = tweens[i].getProgress();
            setColor(0.25f);
            drawRect(trackStartX, y + 18, (trackEndX - trackStartX) * progress, 2);

            // Draw circle
            if (tweens[i].isComplete()) {
                c = c.lerp(colors::white, 0.5f);
            }
            setColor(c);
            drawCircle(animX, y, circleRadius);

            y += rowHeight;
        }

        // Draw instructions
        setColor(0.5f);
        drawBitmapString("Click: Restart animation    |    1: EaseIn  2: EaseOut  3: EaseInOut", 30, getWindowHeight() - 30);
    }

    void drawMiniGraph(float x, float y, float w, float h, EaseType type, const Color& color) {
        // Background
        setColor(0.18f);
        drawRect(x, y, w, h);

        // Draw curve
        setColor(color);
        float prevPx = x;
        float prevPy = y + h;
        int segments = 20;
        for (int j = 1; j <= segments; j++) {
            float t = (float)j / segments;
            float easedT = ease(t, type, currentMode);

            float px = x + t * w;
            float py = y + h - easedT * h;

            drawLine(prevPx, prevPy, px, py);
            prevPx = px;
            prevPy = py;
        }

        // Draw border
        setColor(0.3f);
        drawLine(x, y + h, x + w, y + h);  // bottom
        drawLine(x, y, x, y + h);          // left
    }

    string getModeString(EaseMode mode) {
        switch (mode) {
            case EaseMode::In: return "In";
            case EaseMode::Out: return "Out";
            case EaseMode::InOut: return "InOut";
            default: return "Unknown";
        }
    }

    void mousePressed(Vec2 pos, int button) override {
        initTweens();
    }

    void keyPressed(int key) override {
        if (key == '1') {
            currentMode = EaseMode::In;
            initTweens();
        } else if (key == '2') {
            currentMode = EaseMode::Out;
            initTweens();
        } else if (key == '3') {
            currentMode = EaseMode::InOut;
            initTweens();
        }
    }
};

int main() {
    return runApp<tcApp>(WindowSettings()
        .setSize(960, 800)
        .setTitle("Tween Example"));
}
