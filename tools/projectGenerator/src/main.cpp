#include "TrussC.h"
#include "tcApp.h"

int main() {
    tc::WindowSettings settings;
    settings.title = "TrussC Project Generator";
    settings.width = 500;
    settings.height = 420;
    settings.highDpi = false;
    return tc::runApp<tcApp>(settings);
}
