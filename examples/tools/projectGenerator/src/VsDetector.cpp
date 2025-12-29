#include "VsDetector.h"
#include <algorithm>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

vector<VsVersionInfo> VsDetector::detectInstalledVersions() {
    vector<VsVersionInfo> versions;

#ifdef _WIN32
    // Use vswhere to detect installed Visual Studio versions
    // vswhere is installed with Visual Studio Installer
    string vswhereCmd = R"("C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -all -format value -property installationVersion)";

    FILE* pipe = _popen(vswhereCmd.c_str(), "r");
    if (pipe) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            string version(buffer);
            // Remove newline
            version.erase(version.find_last_not_of("\r\n") + 1);

            if (version.empty()) continue;

            // Parse major version (e.g., "17.5.33424.131" -> 17)
            int majorVersion = 0;
            try {
                majorVersion = stoi(version.substr(0, version.find('.')));
            } catch (...) {
                continue;
            }

            // Check if this version is already added
            bool found = false;
            for (const auto& v : versions) {
                if (v.version == majorVersion) {
                    found = true;
                    break;
                }
            }
            if (found) continue;

            // Create version info
            VsVersionInfo info;
            info.version = majorVersion;

            if (majorVersion == 17) {
                info.displayName = "Visual Studio 2022";
                info.generator = "Visual Studio 17 2022";
            } else if (majorVersion == 18) {
                info.displayName = "Visual Studio 2026";
                info.generator = "Visual Studio 18 2026";
            } else if (majorVersion == 16) {
                info.displayName = "Visual Studio 2019";
                info.generator = "Visual Studio 16 2019";
            } else {
                // Unknown version, skip
                continue;
            }

            versions.push_back(info);
        }
        _pclose(pipe);
    }

    // Sort by version (newest first)
    sort(versions.begin(), versions.end(),
         [](const VsVersionInfo& a, const VsVersionInfo& b) {
             return a.version > b.version;
         });

    // If nothing found, add VS2022 as fallback
    if (versions.empty()) {
        versions.push_back({17, "Visual Studio 2022", "Visual Studio 17 2022"});
    }
#else
    // Not Windows - add dummy entry (won't be used)
    versions.push_back({17, "Visual Studio 2022", "Visual Studio 17 2022"});
#endif

    return versions;
}

tuple<int, int, int> VsDetector::getCmakeVersion() {
    int major = 0, minor = 0, patch = 0;

    FILE* pipe = nullptr;
#ifdef _WIN32
    pipe = _popen("cmake --version", "r");
#else
    pipe = popen("cmake --version", "r");
#endif

    if (pipe) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            // Parse "cmake version X.Y.Z"
            string line(buffer);
            size_t pos = line.find("version ");
            if (pos != string::npos) {
                string ver = line.substr(pos + 8);
                sscanf(ver.c_str(), "%d.%d.%d", &major, &minor, &patch);
            }
        }
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
    }

    return {major, minor, patch};
}

bool VsDetector::checkCmakeVersionForVs(int vsVersion, string& errorMessage) {
    if (vsVersion < 18) {
        // VS2022 and earlier work with CMake 3.x
        return true;
    }

    // VS2026 (version 18) requires CMake 4.2+
    auto [major, minor, patch] = getCmakeVersion();

    if (major > 4) return true;
    if (major == 4 && minor >= 2) return true;

    // Build error message
    errorMessage = "Visual Studio 2026 requires CMake 4.2 or later.\n";
    errorMessage += "Current CMake version in PATH: ";
    if (major == 0) {
        errorMessage += "not found\n";
    } else {
        errorMessage += to_string(major) + "." + to_string(minor) + "." + to_string(patch) + "\n";
    }
    errorMessage += "\nPlease update CMake or ensure the correct version is in your PATH.";

    return false;
}
