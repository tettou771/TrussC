#include "tcApp.h"

int main() {
    tc::WindowSettings settings;
    settings.setSize(960, 600);
    settings.setTitle("tcxQuadWarp Example");
    
    return tc::runApp<tcApp>(settings);
}
