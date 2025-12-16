#pragma once

#include "tcBaseApp.h"
#include <vector>
#include <string>

using namespace trussc;
using namespace std;

// jsonXmlExample - JSON/XML 読み書きのデモ

class tcApp : public tc::App {
public:
    void setup() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    // 表示用メッセージ
    vector<string> messages;

    void addMessage(const string& msg);
    void testJson();
    void testXml();
};
