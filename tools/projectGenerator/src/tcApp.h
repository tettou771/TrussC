#pragma once

#include "tcBaseApp.h"
#include <vector>
#include <string>

using namespace trussc;
using namespace std;

// IDE 選択
enum class IdeType {
    CMakeOnly,
    VSCode,
    Xcode
};

class tcApp : public tc::App {
public:
    void setup() override;
    void draw() override;
    void cleanup() override;

private:
    // 設定
    string tcPath;                      // TC_PATH
    string projectName = "myProject";   // プロジェクト名
    string projectDir;                  // 保存先
    int selectedVersion = 0;            // 選択中のバージョンインデックス
    vector<string> versions;            // 利用可能なバージョン
    vector<string> addons;              // 利用可能なアドオン
    vector<int> addonSelected;          // アドオン選択状態 (0/1)
    IdeType ideType = IdeType::CMakeOnly;

    // UI 状態
    bool showSetupDialog = false;       // TC_PATH 設定ダイアログ
    string statusMessage;               // ステータスメッセージ
    bool statusIsError = false;

    // 設定ファイル
    string configPath;

    // ImGui 用バッファ
    char projectNameBuf[256] = "myProject";
    char projectDirBuf[512] = "";
    char tcPathBuf[512] = "";

    // ヘルパー関数
    void loadConfig();
    void saveConfig();
    void scanVersions();
    void scanAddons();
    bool generateProject();
    void generateVSCodeFiles(const string& path);
    void generateXcodeProject(const string& path);
    string getTemplatePath();
    void setStatus(const string& msg, bool isError = false);
};
