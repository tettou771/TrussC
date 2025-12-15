// =============================================================================
// tcFbo_mac.mm - FBO のプラットフォーム固有実装（macOS / Metal）
// =============================================================================

#include "TrussC.h"

#ifdef __APPLE__
#import <Metal/Metal.h>

namespace trussc {

bool Fbo::readPixelsPlatform(unsigned char* pixels) const {
    if (!allocated_ || !pixels) return false;

    // Metal テクスチャを取得
    sg_mtl_image_info info = sg_mtl_query_image_info(colorImage_);
    id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)info.tex[info.active_slot];

    if (!mtlTexture) return false;

    // ピクセルデータを読み取る
    MTLRegion region = MTLRegionMake2D(0, 0, width_, height_);
    NSUInteger bytesPerRow = width_ * 4;  // RGBA8

    [mtlTexture getBytes:pixels
             bytesPerRow:bytesPerRow
              fromRegion:region
             mipmapLevel:0];

    return true;
}

} // namespace trussc

#endif // __APPLE__
