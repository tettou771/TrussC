// =============================================================================
// main.cpp - TCP ソケットサンプル
// =============================================================================

#include "tcApp.h"

int main() {
    tc::WindowSettings settings;
    settings.setSize(800, 600);
    settings.setTitle("tcpExample - TCP Socket Demo");

    return tc::runApp<tcApp>(settings);
}
