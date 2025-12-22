#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.setHighDpi(false);
    return runApp<tcApp>(settings);
}
