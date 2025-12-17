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
    // サーバーモードで起動
    tc::TcpServer server;

    // クライアントモード用
    tc::TcpClient client;

    // イベントリスナー
    tc::EventListener clientConnectListener;
    tc::EventListener clientDisconnectListener;
    tc::EventListener serverReceiveListener;
    tc::EventListener serverErrorListener;

    tc::EventListener clientConnectedListener;
    tc::EventListener clientReceiveListener;
    tc::EventListener clientDisconnectListener2;
    tc::EventListener clientErrorListener;

    // ログメッセージ
    std::vector<std::string> logMessages;
    std::mutex logMutex;

    // 状態
    bool isServerMode = true;
    int messageCount = 0;

    void addLog(const std::string& msg);
};
