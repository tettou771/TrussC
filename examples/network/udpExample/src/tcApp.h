#pragma once

#include "tcBaseApp.h"

class tcApp : public tc::App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    void cleanup() override;

private:
    tc::UdpSocket receiver;
    tc::UdpSocket sender;

    tc::EventListener receiveListener;
    tc::EventListener errorListener;

    std::vector<std::string> receivedMessages;
    std::mutex messagesMutex;

    int sendCount = 0;
};
