#include "tcApp.h"
#include <filesystem>

void tcApp::setup() {
    // Enable ImGui
    imguiSetup();

    // Setup log listener
    setupLogListener();

    logNotice("TcvEncoder") << "TCV Encoder v4 - ImGui Edition";

    // Initialize default settings (balanced preset)
    settings_.quality = 1;
    settings_.partitions = 16;
    settings_.uber = 1;
    settings_.jobs = 0;

    parseCommandLine();
}

void tcApp::setupLogListener() {
    tcGetLogger().onLog.listen(logListener_, [this](LogEventArgs& e) {
        LogEntry entry;
        entry.level = e.level;
        entry.timestamp = e.timestamp;
        entry.message = e.message;
        logBuffer_.push_back(entry);

        // Limit buffer size
        if (logBuffer_.size() > MAX_LOG_ENTRIES) {
            logBuffer_.erase(logBuffer_.begin(), logBuffer_.begin() + 100);
        }
    });
}

void tcApp::exit() {
    logListener_.disconnect();
    imguiShutdown();
}

void tcApp::parseCommandLine() {
    int argc = getArgCount();
    char** argv = getArgValues();

    if (argc <= 1) {
        logNotice("TcvEncoder") << "Drag & drop a video file to encode";
        return;
    }

    string inputPath;
    string outputPath;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            inputPath = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputPath = argv[++i];
        } else if ((arg == "-q" || arg == "--quality") && i + 1 < argc) {
            string q = argv[++i];
            if (q == "fast" || q == "0") settings_.quality = 0;
            else if (q == "balanced" || q == "1") settings_.quality = 1;
            else if (q == "high" || q == "2") settings_.quality = 2;
        } else if ((arg == "-j" || arg == "--jobs") && i + 1 < argc) {
            settings_.jobs = stoi(argv[++i]);
        } else if (arg == "--partitions" && i + 1 < argc) {
            settings_.partitions = stoi(argv[++i]);
        } else if (arg == "--uber" && i + 1 < argc) {
            settings_.uber = stoi(argv[++i]);
        } else if (arg == "--all-i") {
            settings_.forceAllIFrames = true;
        } else if (arg == "--no-skip") {
            settings_.enableSkip = false;
        } else if (arg == "-h" || arg == "--help") {
            showHelp();
            exitApp();
            return;
        } else if (arg[0] != '-') {
            if (inputPath.empty()) {
                inputPath = arg;
            }
        }
    }

    if (!inputPath.empty()) {
        cliMode_ = true;
        filesToEncode_.push_back(inputPath);
        if (!outputPath.empty()) {
            settings_.outputPath = outputPath;
        }
        startEncoding(inputPath);
    }
}

void tcApp::showHelp() {
    logNotice("TcvEncoder") << "Usage: TrussC_Video_Codec_Encoder -i <input> [-o <output>] [-q <quality>]";
    logNotice("TcvEncoder") << "  -i, --input      Input video file";
    logNotice("TcvEncoder") << "  -o, --output     Output .tcv file";
    logNotice("TcvEncoder") << "  -q, --quality    fast(0), balanced(1), high(2)";
    logNotice("TcvEncoder") << "  -j, --jobs N     Number of threads (0=auto)";
}

void tcApp::update() {
    if (state_ == State::Encoding) {
        session_.update();

        if (session_.isComplete() || session_.hasFailed()) {
            state_ = State::Done;

            // Update output file size
            if (session_.isComplete() && filesystem::exists(session_.getOutputPath())) {
                fileInfo_.outputSize = filesystem::file_size(session_.getOutputPath());
            }
        }
    }

    if (cliMode_ && state_ == State::Done) {
        currentFileIndex_++;
        if (currentFileIndex_ < static_cast<int>(filesToEncode_.size())) {
            startEncoding(filesToEncode_[currentFileIndex_]);
        } else {
            logNotice("TcvEncoder") << "All files encoded";
            exitApp();
        }
    }
}

void tcApp::draw() {
    clear(0.12f);

    if (!cliMode_) {
        // Begin ImGui frame
        imguiBegin();

        drawGui();

        // End ImGui frame
        imguiEnd();
    }
}

void tcApp::drawGui() {
    // Main window covering full app
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(getWindowWidth(), getWindowHeight()));
    ImGui::Begin("TCV Encoder", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Split into left and right panes
    float leftWidth = 300;
    float rightWidth = getWindowWidth() - leftWidth - 20;

    ImGui::BeginChild("LeftPane", ImVec2(leftWidth, 0), true);
    drawLeftPane(leftWidth);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("RightPane", ImVec2(rightWidth, 0), true);
    drawRightPane();
    ImGui::EndChild();

    ImGui::End();
}

void tcApp::drawLeftPane(float width) {
    ImGui::Text("TCV Encoder v4");
    ImGui::Separator();

    // Quality presets
    ImGui::Text("Quality Preset:");
    if (ImGui::Button("Q0 Fast", ImVec2(85, 0))) {
        settings_.partitions = 0;
        settings_.uber = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Q1 Balanced", ImVec2(95, 0))) {
        settings_.partitions = 16;
        settings_.uber = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Q2 High", ImVec2(75, 0))) {
        settings_.partitions = 64;
        settings_.uber = 4;
    }

    // P/U sliders
    ImGui::SliderInt("Partitions", &settings_.partitions, 0, 64);
    ImGui::SliderInt("Uber", &settings_.uber, 0, 4);

    ImGui::Spacing();

    // Advanced settings
    if (ImGui::CollapsingHeader("Advanced Settings")) {
        ImGui::SliderInt("Threads", &settings_.jobs, 0, 16, settings_.jobs == 0 ? "Auto" : "%d");
        ImGui::Checkbox("Force All I-Frames", &settings_.forceAllIFrames);
        ImGui::Checkbox("Enable SKIP", &settings_.enableSkip);
    }

    ImGui::Separator();

    // Encoding status
    ImGui::Text("Status:");
    if (state_ == State::Idle) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Drop a video file to encode");
    } else if (state_ == State::Encoding) {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Encoding...");

        // Progress bar
        float progress = session_.getProgress();
        ImGui::ProgressBar(progress, ImVec2(-1, 0));

        // Frame info
        ImGui::Text("Frame: %d / %d", session_.getCurrentFrame(), session_.getTotalFrames());
        ImGui::Text("Phase: %s", session_.getPhaseString().c_str());
    } else if (state_ == State::Done) {
        if (session_.hasFailed()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Failed!");
        } else {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Complete!");
        }
        ImGui::Text("Encoded %d frames", session_.getEncodedFrames());

        ImGui::Spacing();
        if (ImGui::Button("Encode Another", ImVec2(-1, 0))) {
            state_ = State::Idle;
            fileInfo_ = FileInfo();
        }
    }

    ImGui::Separator();

    // Preview
    if (state_ == State::Encoding && session_.hasSourceTexture()) {
        ImGui::Text("Preview:");
        const Texture& tex = session_.getSourceTexture();
        if (tex.isAllocated()) {
            float previewW = width - 20;
            float aspect = static_cast<float>(tex.getHeight()) / tex.getWidth();
            float previewH = previewW * aspect;
            // Use sokol imgui to display texture
            ImTextureID texId = simgui_imtextureid(tex.getView());
            ImGui::Image(texId, ImVec2(previewW, previewH));
        }
    }

    ImGui::Spacing();

    // Instructions
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Press O to open file dialog");
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Or drag & drop video file");
}

void tcApp::drawRightPane() {
    // File info (top section)
    ImGui::Text("File Information");
    ImGui::Separator();

    if (!fileInfo_.path.empty()) {
        ImGui::Text("Name: %s", fileInfo_.name.c_str());
        ImGui::Text("Size: %dx%d @ %.2f fps", fileInfo_.width, fileInfo_.height, fileInfo_.fps);
        ImGui::Text("Frames: %d", fileInfo_.totalFrames);

        // File sizes
        auto formatSize = [](size_t bytes) -> string {
            if (bytes < 1024) return to_string(bytes) + " B";
            if (bytes < 1024 * 1024) return to_string(bytes / 1024) + " KB";
            return to_string(bytes / (1024 * 1024)) + " MB";
        };

        ImGui::Text("Input: %s", formatSize(fileInfo_.inputSize).c_str());
        if (fileInfo_.outputSize > 0) {
            ImGui::Text("Output: %s", formatSize(fileInfo_.outputSize).c_str());
            float ratio = static_cast<float>(fileInfo_.outputSize) / fileInfo_.inputSize * 100.0f;
            ImGui::Text("Ratio: %.1f%%", ratio);
        }
        ImGui::Text("Output: %s", session_.getOutputPath().c_str());
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No file loaded");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Log window (bottom section, takes remaining space)
    ImGui::Text("Log");
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScrollLog_);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        logBuffer_.clear();
    }

    float logHeight = ImGui::GetContentRegionAvail().y - 10;
    ImGui::BeginChild("LogWindow", ImVec2(0, logHeight), true,
        ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& entry : logBuffer_) {
        ImVec4 color;
        switch (entry.level) {
            case LogLevel::Error:
            case LogLevel::Fatal:
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                break;
            case LogLevel::Warning:
                color = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);
                break;
            case LogLevel::Notice:
                color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
                break;
            default:
                color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
                break;
        }

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s]", entry.timestamp.c_str());
        ImGui::SameLine();
        ImGui::TextColored(color, "%s", entry.message.c_str());
    }

    if (autoScrollLog_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
}

void tcApp::keyPressed(int key) {
    if (key == 'o' || key == 'O') {
        auto result = loadDialog("Select video file");
        if (result.success && !result.filePath.empty()) {
            startEncoding(result.filePath);
        }
    }
}

void tcApp::filesDropped(const vector<string>& files) {
    if (!files.empty() && state_ != State::Encoding) {
        startEncoding(files[0]);
    }
}

void tcApp::startEncoding(const string& inputPath) {
    settings_.inputPath = inputPath;
    settings_.outputPath = getOutputPath(inputPath);

    // Update file info
    fileInfo_.path = inputPath;
    fileInfo_.name = filesystem::path(inputPath).filename().string();
    fileInfo_.outputSize = 0;

    if (filesystem::exists(inputPath)) {
        fileInfo_.inputSize = filesystem::file_size(inputPath);
    }

    if (session_.begin(settings_)) {
        state_ = State::Encoding;

        // Get video info from session
        fileInfo_.width = session_.getVideoWidth();
        fileInfo_.height = session_.getVideoHeight();
        fileInfo_.fps = session_.getVideoFps();
        fileInfo_.totalFrames = session_.getTotalFrames();
    } else {
        logError("TcvEncoder") << "Failed to start encoding";
    }
}

string tcApp::getOutputPath(const string& inputPath) {
    filesystem::path p(inputPath);
    filesystem::path dir = p.parent_path();
    string stem = p.stem().string();
    string ext = ".tcv";

    // Try base name first
    filesystem::path output = dir / (stem + ext);
    if (!filesystem::exists(output)) {
        return output.string();
    }

    // Add suffix if collision
    for (int i = 1; i < 1000; i++) {
        output = dir / (stem + "-" + to_string(i) + ext);
        if (!filesystem::exists(output)) {
            return output.string();
        }
    }

    // Fallback
    return (dir / (stem + "-new" + ext)).string();
}
