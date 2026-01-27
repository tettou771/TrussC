#pragma once

// =============================================================================
// File dialog
// Display OS-native file selection dialog
// =============================================================================
// All dialog functions follow unified parameter order:
//   (title, message, ..., callback for async)
// =============================================================================

#include <string>
#include <vector>
#include <functional>

namespace trussc {

// Dialog result for load/save dialogs
struct FileDialogResult {
    std::string filePath;   // Full path
    std::string fileName;   // Filename only
    bool success = false;   // true if not cancelled
};

// -----------------------------------------------------------------------------
// Alert dialog
// title: Bold header text
// message: Body text
// -----------------------------------------------------------------------------
void alertDialog(const std::string& title, const std::string& message);

void alertDialogAsync(const std::string& title,
                      const std::string& message,
                      std::function<void()> callback = nullptr);

// -----------------------------------------------------------------------------
// Confirm dialog (Yes/No)
// Returns true if user clicked Yes
// -----------------------------------------------------------------------------
bool confirmDialog(const std::string& title, const std::string& message);

void confirmDialogAsync(const std::string& title,
                        const std::string& message,
                        std::function<void(bool)> callback);

// -----------------------------------------------------------------------------
// File open dialog
// folderSelection: true for folder selection mode
// -----------------------------------------------------------------------------
FileDialogResult loadDialog(const std::string& title = "",
                            const std::string& message = "",
                            const std::string& defaultPath = "",
                            bool folderSelection = false);

void loadDialogAsync(const std::string& title,
                     const std::string& message,
                     const std::string& defaultPath,
                     bool folderSelection,
                     std::function<void(const FileDialogResult&)> callback);

// -----------------------------------------------------------------------------
// File save dialog
// defaultName: Initial filename
// -----------------------------------------------------------------------------
FileDialogResult saveDialog(const std::string& title = "",
                            const std::string& message = "",
                            const std::string& defaultPath = "",
                            const std::string& defaultName = "");

void saveDialogAsync(const std::string& title,
                     const std::string& message,
                     const std::string& defaultPath,
                     const std::string& defaultName,
                     std::function<void(const FileDialogResult&)> callback);

} // namespace trussc

// Alias
namespace tc = trussc;
