// =============================================================================
// macOS プラットフォーム固有機能
// =============================================================================

#include "tcPlatform.h"

#if defined(__APPLE__)

#import <Cocoa/Cocoa.h>
#include <CoreGraphics/CoreGraphics.h>

namespace trussc {
namespace platform {

float getDisplayScaleFactor() {
    CGDirectDisplayID displayId = CGMainDisplayID();
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);

    if (!mode) {
        return 1.0f;
    }

    size_t pixelWidth = CGDisplayModeGetPixelWidth(mode);
    size_t pointWidth = CGDisplayModeGetWidth(mode);

    CGDisplayModeRelease(mode);

    if (pointWidth == 0) {
        return 1.0f;
    }

    return (float)pixelWidth / (float)pointWidth;
}

void setWindowSize(int width, int height) {
    // メインウィンドウを取得
    NSWindow* window = [[NSApplication sharedApplication] mainWindow];
    if (!window) {
        // mainWindow が nil の場合、最初のウィンドウを試す
        NSArray* windows = [[NSApplication sharedApplication] windows];
        if (windows.count > 0) {
            window = windows[0];
        }
    }

    if (window) {
        // 現在のフレームを取得
        NSRect frame = [window frame];

        // コンテンツ領域のサイズを変更（タイトルバーは維持）
        NSRect contentRect = [window contentRectForFrameRect:frame];
        contentRect.size.width = width;
        contentRect.size.height = height;

        // 新しいフレームを計算（左上を基準に維持）
        NSRect newFrame = [window frameRectForContentRect:contentRect];
        newFrame.origin.y = frame.origin.y + frame.size.height - newFrame.size.height;

        [window setFrame:newFrame display:YES animate:NO];
    }
}

std::string getExecutablePath() {
    NSString* path = [[NSBundle mainBundle] executablePath];
    return std::string([path UTF8String]);
}

std::string getExecutableDir() {
    NSString* path = [[NSBundle mainBundle] executablePath];
    NSString* dir = [path stringByDeletingLastPathComponent];
    return std::string([dir UTF8String]) + "/";
}

} // namespace platform
} // namespace trussc

#endif // __APPLE__
