#pragma once

#include "tcBaseApp.h"
using namespace tc;

class tcApp : public App {
public:
    void setup() override;
    void draw() override;

private:
    Image tower;
    Image transparency;
};
