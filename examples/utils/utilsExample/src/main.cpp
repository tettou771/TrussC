#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.title = "Utils Example";
    settings.width = 750;
    settings.height = 580;

    return runApp<tcApp>(settings);
}
