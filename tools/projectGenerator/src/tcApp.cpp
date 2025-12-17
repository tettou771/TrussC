// =============================================================================
// tcApp.cpp - TrussC Project Generator
// =============================================================================

#include "tcApp.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void tcApp::setup() {
    imguiSetup();

    // 設定ファイルパス (~/.trussc/config.json)
    string home = getenv("HOME") ? getenv("HOME") : "";
    configPath = home + "/.trussc/config.json";

    // 設定を読み込み
    loadConfig();

    // TC_PATH が未設定なら環境変数から取得
    if (tcPath.empty()) {
        const char* env = getenv("TC_PATH");
        if (env) {
            tcPath = env;
        }
    }

    // TC_PATH が設定されていない場合はダイアログを表示
    if (tcPath.empty()) {
        showSetupDialog = true;
    } else {
        strncpy(tcPathBuf, tcPath.c_str(), sizeof(tcPathBuf) - 1);
        scanVersions();
    }

    // デフォルトの保存先
    if (projectDir.empty()) {
        projectDir = home + "/Projects";
    }
    strncpy(projectDirBuf, projectDir.c_str(), sizeof(projectDirBuf) - 1);
}

void tcApp::draw() {
    clear(45, 45, 48);

    imguiBegin();

    // TC_PATH 設定ダイアログ
    if (showSetupDialog) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(getWindowWidth(), getWindowHeight()));
        ImGui::Begin("Setup TC_PATH", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        ImGui::Spacing();
        ImGui::Text("Setup TC_PATH");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("TC_PATH environment variable is not set.");
        ImGui::TextWrapped("Please select the TrussC installation directory.");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text("TC_PATH");
        ImGui::SetNextItemWidth(-80);
        ImGui::InputText("##tcPath", tcPathBuf, sizeof(tcPathBuf));
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            auto result = loadDialog("Select TrussC directory", true);
            if (result.success) {
                strncpy(tcPathBuf, result.filePath.c_str(), sizeof(tcPathBuf) - 1);
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 30))) {
            tcPath = tcPathBuf;
            if (!tcPath.empty() && fs::exists(tcPath)) {
                showSetupDialog = false;
                saveConfig();
                scanVersions();
            } else {
                setStatus("Invalid path", true);
            }
        }

        // ステータスメッセージ
        if (!statusMessage.empty() && statusIsError) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", statusMessage.c_str());
        }

        ImGui::End();
        imguiEnd();
        return;  // メインウィンドウは描画しない
    }

    // メインウィンドウ（ウィンドウ全体に固定）
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(getWindowWidth(), getWindowHeight()));
    ImGui::Begin("TrussC Project Generator", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    // プロジェクト名
    ImGui::Text("Project Name");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##projectName", projectNameBuf, sizeof(projectNameBuf));

    ImGui::Spacing();

    // 保存先
    ImGui::Text("Location");
    ImGui::SetNextItemWidth(-80);
    ImGui::InputText("##projectDir", projectDirBuf, sizeof(projectDirBuf));
    ImGui::SameLine();
    if (ImGui::Button("Browse##dir")) {
        auto result = loadDialog("Select project location", true);
        if (result.success) {
            strncpy(projectDirBuf, result.filePath.c_str(), sizeof(projectDirBuf) - 1);
            projectDir = projectDirBuf;
            saveConfig();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // バージョン選択
    ImGui::Text("TrussC Version");
    if (!versions.empty()) {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##version", versions[selectedVersion].c_str())) {
            for (int i = 0; i < (int)versions.size(); i++) {
                bool isSelected = (selectedVersion == i);
                if (ImGui::Selectable(versions[i].c_str(), isSelected)) {
                    selectedVersion = i;
                    scanAddons();
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    } else {
        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "No versions found");
    }

    ImGui::Spacing();

    // アドオン選択
    ImGui::Text("Addons");
    ImGui::BeginChild("##addons", ImVec2(0, 100), true);
    if (addons.empty()) {
        ImGui::TextDisabled("No addons available");
    } else {
        for (size_t i = 0; i < addons.size(); i++) {
            bool selected = addonSelected[i] != 0;
            if (ImGui::Checkbox(addons[i].c_str(), &selected)) {
                addonSelected[i] = selected ? 1 : 0;
            }
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();

    // IDE 選択
    ImGui::Text("IDE");
    ImGui::SetNextItemWidth(-1);
    const char* ideItems[] = { "CMake only", "VSCode", "Xcode (macOS)" };
    int ideIndex = static_cast<int>(ideType);
    if (ImGui::Combo("##ide", &ideIndex, ideItems, 3)) {
        ideType = static_cast<IdeType>(ideIndex);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 生成ボタン
    if (ImGui::Button("Generate Project", ImVec2(-1, 40))) {
        projectName = projectNameBuf;
        projectDir = projectDirBuf;
        if (generateProject()) {
            setStatus("Project created successfully!");
        }
    }

    // ステータスメッセージ
    if (!statusMessage.empty()) {
        ImGui::Spacing();
        if (statusIsError) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", statusMessage.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.4f, 1, 0.4f, 1), "%s", statusMessage.c_str());
        }
    }

    // TC_PATH 設定ボタン（下部）
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 35);
    ImGui::Separator();
    if (ImGui::SmallButton("Settings...")) {
        showSetupDialog = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("TC_PATH: %s", tcPath.c_str());

    ImGui::End();

    imguiEnd();
}

void tcApp::cleanup() {
    imguiShutdown();
}

void tcApp::loadConfig() {
    if (!fs::exists(configPath)) return;

    Json config = loadJson(configPath);
    if (config.empty()) return;

    if (config.contains("tc_path")) {
        tcPath = config["tc_path"].get<string>();
    }
    if (config.contains("last_project_dir")) {
        projectDir = config["last_project_dir"].get<string>();
    }
}

void tcApp::saveConfig() {
    // ~/.trussc/ ディレクトリを作成
    fs::path configDir = fs::path(configPath).parent_path();
    if (!fs::exists(configDir)) {
        fs::create_directories(configDir);
    }

    Json config;
    config["tc_path"] = tcPath;
    config["last_project_dir"] = projectDir;
    saveJson(config, configPath);
}

void tcApp::scanVersions() {
    versions.clear();
    selectedVersion = 0;

    if (tcPath.empty() || !fs::exists(tcPath)) return;

    // tc_v* フォルダを列挙
    for (const auto& entry : fs::directory_iterator(tcPath)) {
        if (entry.is_directory()) {
            string name = entry.path().filename().string();
            if (name.substr(0, 4) == "tc_v") {
                versions.push_back(name);
            }
        }
    }

    // ソート（新しいバージョン順）
    sort(versions.rbegin(), versions.rend());

    if (!versions.empty()) {
        scanAddons();
    }
}

void tcApp::scanAddons() {
    addons.clear();
    addonSelected.clear();

    if (versions.empty()) return;

    string addonsPath = tcPath + "/" + versions[selectedVersion] + "/addons";
    if (!fs::exists(addonsPath)) return;

    // アドオンフォルダを列挙
    for (const auto& entry : fs::directory_iterator(addonsPath)) {
        if (entry.is_directory()) {
            string name = entry.path().filename().string();
            if (name.substr(0, 3) == "tcx") {
                addons.push_back(name);
                addonSelected.push_back(0);
            }
        }
    }

    // ソート
    sort(addons.begin(), addons.end());
}

string tcApp::getTemplatePath() {
    if (versions.empty()) return "";
    return tcPath + "/" + versions[selectedVersion] + "/examples/templates/emptyExample";
}

bool tcApp::generateProject() {
    // バリデーション
    if (projectName.empty()) {
        setStatus("Project name is required", true);
        return false;
    }

    if (projectDir.empty()) {
        setStatus("Location is required", true);
        return false;
    }

    if (versions.empty()) {
        setStatus("No TrussC version available", true);
        return false;
    }

    string templatePath = getTemplatePath();
    if (!fs::exists(templatePath)) {
        setStatus("Template not found", true);
        return false;
    }

    // プロジェクトパス（末尾のスラッシュを除去）
    while (!projectDir.empty() && projectDir.back() == '/') {
        projectDir.pop_back();
    }
    string destPath = projectDir + "/" + projectName;

    // 既存チェック
    if (fs::exists(destPath)) {
        setStatus("Project already exists", true);
        return false;
    }

    try {
        // 親ディレクトリを作成
        fs::create_directories(projectDir);

        // テンプレートをコピー（build, bin フォルダは除外）
        fs::create_directories(destPath);
        for (const auto& entry : fs::directory_iterator(templatePath)) {
            string name = entry.path().filename().string();
            if (name == "build" || name == "bin") {
                continue;  // スキップ
            }
            fs::copy(entry.path(), destPath / entry.path().filename(),
                     fs::copy_options::recursive);
        }

        // CMakeLists.txt を書き換え
        string cmakePath = destPath + "/CMakeLists.txt";
        ifstream inFile(cmakePath);
        stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();

        string content = buffer.str();

        // バージョン書き換え（tc_v0.0.1 → 選択したバージョン）
        string versionNum = versions[selectedVersion].substr(4); // "tc_v0.0.1" → "0.0.1"
        size_t pos = content.find("TC_VERSION \"0.0.1\"");
        if (pos != string::npos) {
            content.replace(pos, 18, "TC_VERSION \"" + versionNum + "\"");
        }

        // アドオン追加
        string addonLines;
        for (size_t i = 0; i < addons.size(); i++) {
            if (addonSelected[i]) {
                addonLines += "use_addon(${PROJECT_NAME} " + addons[i] + ")\n";
            }
        }

        if (!addonLines.empty()) {
            // コメントアウトされた use_addon を置き換え
            pos = content.find("# use_addon(${PROJECT_NAME} tcxBox2d)");
            if (pos != string::npos) {
                content.replace(pos, 37, addonLines);
            }
        }

        // 書き戻し
        ofstream outFile(cmakePath);
        outFile << content;
        outFile.close();

        // IDE 固有のファイル生成
        if (ideType == IdeType::VSCode) {
            generateVSCodeFiles(destPath);
        } else if (ideType == IdeType::Xcode) {
            generateXcodeProject(destPath);
        }

        saveConfig();
        return true;

    } catch (const exception& e) {
        setStatus(string("Error: ") + e.what(), true);
        return false;
    }
}

void tcApp::generateVSCodeFiles(const string& path) {
    // .vscode フォルダを作成
    string vscodePath = path + "/.vscode";
    fs::create_directories(vscodePath);

    // launch.json
    Json launch;
    launch["version"] = "0.2.0";
    launch["configurations"] = Json::array();

    Json config;
    config["name"] = "Debug";
    config["type"] = "lldb";
    config["request"] = "launch";
    config["program"] = "${workspaceFolder}/bin/" + projectName + ".app/Contents/MacOS/" + projectName;
    config["cwd"] = "${workspaceFolder}";
    config["preLaunchTask"] = "CMake: build";

    launch["configurations"].push_back(config);
    saveJson(launch, vscodePath + "/launch.json");

    // settings.json
    Json settings;
    settings["cmake.buildDirectory"] = "${workspaceFolder}/build";
    settings["cmake.sourceDirectory"] = "${workspaceFolder}";
    saveJson(settings, vscodePath + "/settings.json");
}

void tcApp::generateXcodeProject(const string& path) {
    // build フォルダを作成して cmake -G Xcode を実行
    string buildPath = path + "/build";
    fs::create_directories(buildPath);

    string cmd = "cd \"" + buildPath + "\" && cmake -G Xcode ..";
    int result = system(cmd.c_str());

    if (result != 0) {
        tcLogWarning() << "Failed to generate Xcode project";
    }
}

void tcApp::setStatus(const string& msg, bool isError) {
    statusMessage = msg;
    statusIsError = isError;
}
