#pragma once

#include "tcBaseApp.h"

class tcApp : public tc::App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    tc::VideoGrabber grabber_;
    std::vector<tc::VideoDeviceInfo> devices_;
    int currentDevice_ = 0;
    bool permissionGranted_ = false;
    bool permissionRequested_ = false;
    int frameCount_ = 0;
    int newFrameCount_ = 0;
};
