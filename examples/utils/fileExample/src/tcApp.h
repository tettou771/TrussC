#pragma once

#include <TrussC.h>
#include <iostream>
using namespace std;
using namespace tc;

class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    // Path test results
    string testPath_ = "data/logs/test.txt";

    // File system test results
    bool dataExists_ = false;
    bool logsExists_ = false;
    vector<string> dirContents_;

    // FileWriter test
    FileWriter logWriter_;
    int logCount_ = 0;
    string lastLogMessage_;

    // FileReader test
    vector<string> readLines_;
    int totalLinesRead_ = 0;
};
