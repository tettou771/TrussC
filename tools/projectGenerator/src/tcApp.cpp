// =============================================================================
// tcApp.cpp - TrussC Project Generator
// =============================================================================

#include "tcApp.h"
#include <filesystem>
#include <fstream>
#include <thread>

namespace fs = std::filesystem;

void tcApp::setup() {
    imguiSetup();

    // 省電力モード: update は 30fps、draw はイベント駆動
    tc::setUpdateFps(30);
    tc::setDrawFps(0);  // 自動描画停止

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

    // 前回のプロジェクトが既存なら自動で Update モードに
    if (!projectDir.empty() && !projectName.empty()) {
        string lastProjectPath = projectDir + "/" + projectName;
        if (fs::exists(lastProjectPath + "/CMakeLists.txt")) {
            importProject(lastProjectPath);
        }
    }

    // 初回描画
    tc::redraw();
}

void tcApp::update() {
    // 生成中は毎フレーム再描画（明滅アニメーション用）
    if (isGenerating) {
        tc::redraw();
    }
}

// マウスイベントで再描画
void tcApp::mousePressed(int x, int y, int button) {
    tc::redraw();
}

void tcApp::mouseReleased(int x, int y, int button) {
    tc::redraw();
}

void tcApp::mouseMoved(int x, int y) {
    tc::redraw();
}

void tcApp::mouseDragged(int x, int y, int button) {
    tc::redraw();
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
    ImGui::SetNextItemWidth(-80);
    if (isImportedProject) {
        ImGui::BeginDisabled();
    }
    ImGui::InputText("##projectName", projectNameBuf, sizeof(projectNameBuf));
    if (isImportedProject) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (ImGui::Button("Import")) {
        auto result = loadDialog("Select existing project", true);
        if (result.success) {
            importProject(result.filePath);
        }
    }

    ImGui::Spacing();

    // 保存先
    ImGui::Text("Location");
    ImGui::SetNextItemWidth(-80);
    if (isImportedProject) {
        ImGui::BeginDisabled();
    }
    ImGui::InputText("##projectDir", projectDirBuf, sizeof(projectDirBuf));
    if (isImportedProject) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (isImportedProject) {
        if (ImGui::Button("New")) {
            resetToNewProject();
        }
    } else {
        if (ImGui::Button("Browse##dir")) {
            auto result = loadDialog("Select project location", true);
            if (result.success) {
                strncpy(projectDirBuf, result.filePath.c_str(), sizeof(projectDirBuf) - 1);
                projectDir = projectDirBuf;
                saveConfig();
            }
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
        saveConfig();  // IDE 変更時に保存
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 生成/更新ボタン
    if (isGenerating) {
        // 生成中: 明滅するボタン
        float t = tc::getElapsedTime();
        float pulse = 0.5f + 0.3f * sinf(t * 4.0f);  // 明滅
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, pulse));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.4f, 0.8f, pulse));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.8f, pulse));
        ImGui::Button("Generating...", ImVec2(-1, 40));
        ImGui::PopStyleColor(3);
    } else if (isImportedProject) {
        if (ImGui::Button("Update Project", ImVec2(-1, 40))) {
            startUpdate();
        }
    } else {
        if (ImGui::Button("Generate Project", ImVec2(-1, 40))) {
            projectName = projectNameBuf;
            projectDir = projectDirBuf;
            startGenerate();
        }
    }

    // 生成ログ表示
    if (isGenerating || !generatingLog.empty()) {
        ImGui::Spacing();
        ImGui::BeginChild("##log", ImVec2(0, 85), true);
        lock_guard<mutex> lock(logMutex);
        ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 10);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", generatingLog.c_str());
        ImGui::PopTextWrapPos();
        // 自動スクロール（一番下へ）
        if (isGenerating) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
    }

    // ステータスメッセージ（ウィンドウ幅-10pxで折り返し）
    if (!statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 10);
        if (statusIsError) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", statusMessage.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.4f, 1, 0.4f, 1), "%s", statusMessage.c_str());
        }
        ImGui::PopTextWrapPos();
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
    if (config.contains("last_project_name")) {
        projectName = config["last_project_name"].get<string>();
        strncpy(projectNameBuf, projectName.c_str(), sizeof(projectNameBuf) - 1);
    }
    if (config.contains("ide_type")) {
        ideType = static_cast<IdeType>(config["ide_type"].get<int>());
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
    config["last_project_name"] = projectName;
    config["ide_type"] = static_cast<int>(ideType);
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

        // CMakeLists.txt を書き換え（TC_ROOT を直接設定）
        string cmakePath = destPath + "/CMakeLists.txt";
        ifstream inFile(cmakePath);
        stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();

        string content = buffer.str();

        // TC_ROOT を直接設定（環境変数不要に）
        string tcRoot = tcPath + "/" + versions[selectedVersion];
        size_t pos = content.find("set(TC_ROOT \"\"");
        if (pos != string::npos) {
            content.replace(pos, 14, "set(TC_ROOT \"" + tcRoot + "\"");
        }

        // 書き戻し
        ofstream outFile(cmakePath);
        outFile << content;
        outFile.close();

        // addons.make を生成
        string addonsMakePath = destPath + "/addons.make";
        ofstream addonsFile(addonsMakePath);
        addonsFile << "# TrussC addons - one addon per line\n";
        for (size_t i = 0; i < addons.size(); i++) {
            if (addonSelected[i]) {
                addonsFile << addons[i] << "\n";
            }
        }
        addonsFile.close();

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
    // build フォルダを削除して cmake -G Xcode を実行
    // （既存の CMakeCache があると Xcode ジェネレータに切り替えられないため）
    string buildPath = path + "/build";
    if (fs::exists(buildPath)) {
        fs::remove_all(buildPath);
    }
    fs::create_directories(buildPath);

    // GUI アプリから実行すると PATH が通ってないので cmake のフルパスを使用
    // TC_ROOT は CMakeLists.txt に直接書いてあるので環境変数不要
    string cmd = "cd \"" + buildPath + "\" && /opt/homebrew/bin/cmake -G Xcode ..";
    tcLog() << "Xcode cmd: " << cmd;
    int result = system(cmd.c_str());

    if (result != 0) {
        tcLogWarning() << "Failed to generate Xcode project (exit code: " << result << ")";
    }
}

void tcApp::setStatus(const string& msg, bool isError) {
    statusMessage = msg;
    statusIsError = isError;
}

void tcApp::importProject(const string& path) {
    // CMakeLists.txt が存在するか確認
    string cmakePath = path + "/CMakeLists.txt";
    if (!fs::exists(cmakePath)) {
        setStatus("Not a valid TrussC project (CMakeLists.txt not found)", true);
        return;
    }

    // CMakeLists.txt を解析
    ifstream inFile(cmakePath);
    stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();
    string content = buffer.str();

    // プロジェクト名を取得（フォルダ名）
    projectName = fs::path(path).filename().string();
    strncpy(projectNameBuf, projectName.c_str(), sizeof(projectNameBuf) - 1);

    // 保存先を設定
    projectDir = fs::path(path).parent_path().string();
    strncpy(projectDirBuf, projectDir.c_str(), sizeof(projectDirBuf) - 1);

    // バージョンを解析 (TC_ROOT からバージョンを抽出)
    // 形式: set(TC_ROOT "/path/to/tc_v0.0.1"
    size_t pos = content.find("set(TC_ROOT \"");
    if (pos != string::npos) {
        size_t start = pos + 13;
        size_t end = content.find("\"", start);
        if (end != string::npos) {
            string tcRootPath = content.substr(start, end - start);
            // パスから tc_vX.X.X を抽出
            size_t lastSlash = tcRootPath.rfind('/');
            if (lastSlash != string::npos) {
                string version = tcRootPath.substr(lastSlash + 1);
                for (int i = 0; i < (int)versions.size(); i++) {
                    if (versions[i] == version) {
                        selectedVersion = i;
                        scanAddons();
                        break;
                    }
                }
            }
        }
    }

    // addons.make からアドオンを読み込み
    for (size_t i = 0; i < addons.size(); i++) {
        addonSelected[i] = 0;
    }

    string addonsMakePath = path + "/addons.make";
    if (fs::exists(addonsMakePath)) {
        ifstream addonsFile(addonsMakePath);
        string line;
        while (getline(addonsFile, line)) {
            // 空白を除去
            size_t start = line.find_first_not_of(" \t");
            if (start == string::npos) continue;
            size_t end = line.find_last_not_of(" \t\r\n");
            string addonName = line.substr(start, end - start + 1);

            // コメント行をスキップ
            if (addonName.empty() || addonName[0] == '#') continue;

            // アドオンリストから探して選択
            for (size_t i = 0; i < addons.size(); i++) {
                if (addons[i] == addonName) {
                    addonSelected[i] = 1;
                    break;
                }
            }
        }
        addonsFile.close();
    }

    // インポート状態を設定
    isImportedProject = true;
    importedProjectPath = path;
    setStatus("Project imported: " + projectName);
}

bool tcApp::updateProject() {
    if (!isImportedProject || importedProjectPath.empty()) {
        setStatus("No project imported", true);
        return false;
    }

    string cmakePath = importedProjectPath + "/CMakeLists.txt";
    if (!fs::exists(cmakePath)) {
        setStatus("CMakeLists.txt not found", true);
        return false;
    }

    try {
        // テンプレートから新しい CMakeLists.txt を読み込み
        string templatePath = getTemplatePath();
        string templateCmakePath = templatePath + "/CMakeLists.txt";

        ifstream inFile(templateCmakePath);
        stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();
        string content = buffer.str();

        // TC_ROOT を直接設定
        string tcRoot = tcPath + "/" + versions[selectedVersion];
        size_t pos = content.find("set(TC_ROOT \"\"");
        if (pos != string::npos) {
            content.replace(pos, 14, "set(TC_ROOT \"" + tcRoot + "\"");
        }

        // 書き戻し
        ofstream outFile(cmakePath);
        outFile << content;
        outFile.close();

        // addons.make を更新
        string addonsMakePath = importedProjectPath + "/addons.make";
        ofstream addonsFile(addonsMakePath);
        addonsFile << "# TrussC addons - one addon per line\n";
        for (size_t i = 0; i < addons.size(); i++) {
            if (addonSelected[i]) {
                addonsFile << addons[i] << "\n";
            }
        }
        addonsFile.close();

        // IDE 固有のファイル生成
        if (ideType == IdeType::VSCode) {
            generateVSCodeFiles(importedProjectPath);
        } else if (ideType == IdeType::Xcode) {
            generateXcodeProject(importedProjectPath);
        }

        return true;

    } catch (const exception& e) {
        setStatus(string("Error: ") + e.what(), true);
        return false;
    }
}

void tcApp::resetToNewProject() {
    isImportedProject = false;
    importedProjectPath = "";
    projectName = "myProject";
    strncpy(projectNameBuf, projectName.c_str(), sizeof(projectNameBuf) - 1);

    // アドオン選択をリセット
    for (size_t i = 0; i < addonSelected.size(); i++) {
        addonSelected[i] = 0;
    }

    setStatus("");
}

void tcApp::startGenerate() {
    if (isGenerating) return;

    isGenerating = true;
    setStatus("");  // 前回のステータスをクリア
    {
        lock_guard<mutex> lock(logMutex);
        generatingLog = "Starting project generation...";
    }

    // スレッドで実行
    thread([this]() {
        doGenerateProject();
        isGenerating = false;
    }).detach();
}

void tcApp::startUpdate() {
    if (isGenerating) return;

    isGenerating = true;
    setStatus("");  // 前回のステータスをクリア
    {
        lock_guard<mutex> lock(logMutex);
        generatingLog = "Starting project update...";
    }

    // スレッドで実行
    thread([this]() {
        doUpdateProject();
        isGenerating = false;
    }).detach();
}

void tcApp::doGenerateProject() {
    auto log = [this](const string& msg) {
        lock_guard<mutex> lock(logMutex);
        generatingLog += msg + "\n";
        tc::redraw();
    };

    // バリデーション
    if (projectName.empty()) {
        log("Error: Project name is required");
        setStatus("Project name is required", true);
        return;
    }

    if (projectDir.empty()) {
        log("Error: Location is required");
        setStatus("Location is required", true);
        return;
    }

    if (versions.empty()) {
        log("Error: No TrussC version available");
        setStatus("No TrussC version available", true);
        return;
    }

    string templatePath = getTemplatePath();
    if (!fs::exists(templatePath)) {
        log("Error: Template not found");
        setStatus("Template not found", true);
        return;
    }

    // プロジェクトパス（末尾のスラッシュを除去）
    string projDir = projectDir;
    while (!projDir.empty() && projDir.back() == '/') {
        projDir.pop_back();
    }
    string destPath = projDir + "/" + projectName;

    // 既存チェック
    if (fs::exists(destPath)) {
        log("Error: Project already exists");
        setStatus("Project already exists", true);
        return;
    }

    try {
        log("Creating project directory...");

        // 親ディレクトリを作成
        fs::create_directories(projDir);

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

        log("Configuring CMakeLists.txt...");

        // CMakeLists.txt を書き換え（TC_ROOT を直接設定）
        string cmakePath = destPath + "/CMakeLists.txt";
        ifstream inFile(cmakePath);
        stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();

        string content = buffer.str();

        // TC_ROOT を直接設定（環境変数不要に）
        string tcRoot = tcPath + "/" + versions[selectedVersion];
        size_t pos = content.find("set(TC_ROOT \"\"");
        if (pos != string::npos) {
            content.replace(pos, 14, "set(TC_ROOT \"" + tcRoot + "\"");
        }

        // 書き戻し
        ofstream outFile(cmakePath);
        outFile << content;
        outFile.close();

        log("Creating addons.make...");

        // addons.make を生成
        string addonsMakePath = destPath + "/addons.make";
        ofstream addonsFile(addonsMakePath);
        addonsFile << "# TrussC addons - one addon per line\n";
        for (size_t i = 0; i < addons.size(); i++) {
            if (addonSelected[i]) {
                addonsFile << addons[i] << "\n";
            }
        }
        addonsFile.close();

        // IDE 固有のファイル生成
        if (ideType == IdeType::VSCode) {
            log("Generating VSCode files...");
            generateVSCodeFiles(destPath);
        } else if (ideType == IdeType::Xcode) {
            log("Generating Xcode project (this may take a while)...");
            generateXcodeProject(destPath);
        }

        log("Done!");
        saveConfig();
        setStatus("Project created successfully!");
        tc::redraw();  // 完了時に再描画

    } catch (const exception& e) {
        log(string("Error: ") + e.what());
        setStatus(string("Error: ") + e.what(), true);
        tc::redraw();  // エラー時も再描画
    }
}

void tcApp::doUpdateProject() {
    auto log = [this](const string& msg) {
        lock_guard<mutex> lock(logMutex);
        generatingLog += msg + "\n";
        tc::redraw();
    };

    if (!isImportedProject || importedProjectPath.empty()) {
        log("Error: No project imported");
        setStatus("No project imported", true);
        return;
    }

    string cmakePath = importedProjectPath + "/CMakeLists.txt";
    if (!fs::exists(cmakePath)) {
        log("Error: CMakeLists.txt not found");
        setStatus("CMakeLists.txt not found", true);
        return;
    }

    try {
        log("Reading template...");

        // テンプレートから新しい CMakeLists.txt を読み込み
        string templatePath = getTemplatePath();
        string templateCmakePath = templatePath + "/CMakeLists.txt";

        ifstream inFile(templateCmakePath);
        stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();
        string content = buffer.str();

        log("Configuring CMakeLists.txt...");

        // TC_ROOT を直接設定
        string tcRoot = tcPath + "/" + versions[selectedVersion];
        size_t pos = content.find("set(TC_ROOT \"\"");
        if (pos != string::npos) {
            content.replace(pos, 14, "set(TC_ROOT \"" + tcRoot + "\"");
        }

        // 書き戻し
        ofstream outFile(cmakePath);
        outFile << content;
        outFile.close();

        log("Updating addons.make...");

        // addons.make を更新
        string addonsMakePath = importedProjectPath + "/addons.make";
        ofstream addonsFile(addonsMakePath);
        addonsFile << "# TrussC addons - one addon per line\n";
        for (size_t i = 0; i < addons.size(); i++) {
            if (addonSelected[i]) {
                addonsFile << addons[i] << "\n";
            }
        }
        addonsFile.close();

        // IDE 固有のファイル生成
        if (ideType == IdeType::VSCode) {
            log("Generating VSCode files...");
            generateVSCodeFiles(importedProjectPath);
        } else if (ideType == IdeType::Xcode) {
            log("Generating Xcode project (this may take a while)...");
            generateXcodeProject(importedProjectPath);
        }

        log("Done!");
        setStatus("Project updated successfully!");
        tc::redraw();  // 完了時に再描画

    } catch (const exception& e) {
        log(string("Error: ") + e.what());
        setStatus(string("Error: ") + e.what(), true);
        tc::redraw();  // エラー時も再描画
    }
}
