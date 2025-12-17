#include "tcApp.h"

int main() {
    tc::WindowSettings settings;
    settings.width = 800;
    settings.height = 600;
    settings.title = "timerExample";

    tc::runApp<tcApp>(settings);
    return 0;
}
