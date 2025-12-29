// =============================================================================
// main.cpp - Entry point for noWindowMode example
// =============================================================================

#include "tcApp.h"

int main() {
    tc::HeadlessSettings settings;
    settings.setFps(60.0f);

    return tc::runHeadlessApp<tcApp>(settings);
}
