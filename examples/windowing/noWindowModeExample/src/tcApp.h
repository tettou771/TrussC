#pragma once

#include <TrussC.h>
using namespace std;
using namespace tc;

class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void cleanup() override;

private:
    uint64_t frameCount_ = 0;
};
