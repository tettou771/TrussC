// =============================================================================
// videoGrabberExample - Webカメラ入力サンプル
// =============================================================================

#include "tcApp.h"

int main() {
    tc::WindowSettings settings;
    settings.setSize(800, 600);
    settings.setTitle("videoGrabberExample - TrussC");

    return tc::runApp<tcApp>(settings);
}
