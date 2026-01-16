#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.setSize(800, 600);
    settings.title = "File Example";

    return runApp<tcApp>(settings);
}
