#pragma once

// =============================================================================
// tcVideoGrabber.h - Webカメラ入力
// =============================================================================

// このファイルは TrussC.h からインクルードされる

#include <vector>
#include <string>
#include <atomic>
#include <mutex>

namespace trussc {

// ---------------------------------------------------------------------------
// VideoDeviceInfo - カメラデバイス情報
// ---------------------------------------------------------------------------
struct VideoDeviceInfo {
    int deviceId = -1;
    std::string deviceName;
    std::string uniqueId;

    int getDeviceID() const { return deviceId; }
    const std::string& getDeviceName() const { return deviceName; }
    const std::string& getUniqueId() const { return uniqueId; }
};

// ---------------------------------------------------------------------------
// VideoGrabber - Webカメラ入力クラス
// ---------------------------------------------------------------------------
class VideoGrabber {
public:
    VideoGrabber() = default;
    ~VideoGrabber() { close(); }

    // コピー禁止
    VideoGrabber(const VideoGrabber&) = delete;
    VideoGrabber& operator=(const VideoGrabber&) = delete;

    // ムーブ対応
    VideoGrabber(VideoGrabber&& other) noexcept {
        moveFrom(std::move(other));
    }

    VideoGrabber& operator=(VideoGrabber&& other) noexcept {
        if (this != &other) {
            close();
            moveFrom(std::move(other));
        }
        return *this;
    }

    // =========================================================================
    // デバイス管理
    // =========================================================================

    // 利用可能なカメラ一覧を取得
    std::vector<VideoDeviceInfo> listDevices() {
        return listDevicesPlatform();
    }

    // 使用するカメラを指定（setup() の前に呼ぶ）
    void setDeviceID(int deviceId) {
        deviceId_ = deviceId;
    }

    int getDeviceID() const {
        return deviceId_;
    }

    // =========================================================================
    // セットアップ / クローズ
    // =========================================================================

    // カメラを開始（デフォルト 640x480）
    bool setup(int width = 640, int height = 480) {
        if (initialized_) {
            close();
        }

        requestedWidth_ = width;
        requestedHeight_ = height;

        // プラットフォーム固有のセットアップ（width_, height_ が設定される）
        if (!setupPlatform()) {
            return false;
        }

        // ピクセルバッファを確保
        size_t bufferSize = width_ * height_ * 4;
        pixels_ = new unsigned char[bufferSize];
        std::memset(pixels_, 0, bufferSize);

        // デリゲートにピクセルバッファのポインタを設定
        updateDelegatePixels();

        // テクスチャを作成
        createTexture();

        initialized_ = true;
        return true;
    }

    // カメラを停止
    void close() {
        if (!initialized_) return;

        closePlatform();

        if (textureValid_) {
            sg_destroy_sampler(sampler_);
            sg_destroy_view(view_);
            sg_destroy_image(texture_);
            textureValid_ = false;
        }

        if (pixels_) {
            delete[] pixels_;
            pixels_ = nullptr;
        }

        initialized_ = false;
        frameNew_ = false;
        width_ = 0;
        height_ = 0;
    }

    // =========================================================================
    // フレーム更新
    // =========================================================================

    // 新しいフレームをチェック（毎フレーム呼ぶ）
    void update() {
        if (!initialized_) return;

        frameNew_ = false;

        // プラットフォーム固有の更新処理
        updatePlatform();

        // カメラからのサイズ変更通知をチェック
        if (checkResizeNeeded()) {
            int newW = 0, newH = 0;
            getNewSize(newW, newH);
            if (newW > 0 && newH > 0 && (newW != width_ || newH != height_)) {
                // メインスレッドでバッファをリサイズ
                resizeBuffers(newW, newH);
            }
            clearResizeFlag();
        }

        // バッファが更新されていたらテクスチャに反映
        if (pixelsDirty_.exchange(false)) {
            std::lock_guard<std::mutex> lock(mutex_);
            updateTexture();
            frameNew_ = true;
        }
    }

    // 新しいフレームが来たか
    bool isFrameNew() const {
        return frameNew_;
    }

    // =========================================================================
    // 描画
    // =========================================================================

    void draw(float x, float y) const {
        if (!initialized_ || !textureValid_) return;
        drawInternal(x, y, (float)width_, (float)height_);
    }

    void draw(float x, float y, float w, float h) const {
        if (!initialized_ || !textureValid_) return;
        drawInternal(x, y, w, h);
    }

    // =========================================================================
    // 状態取得
    // =========================================================================

    bool isInitialized() const { return initialized_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // =========================================================================
    // ピクセルアクセス
    // =========================================================================

    unsigned char* getPixels() { return pixels_; }
    const unsigned char* getPixels() const { return pixels_; }

    // Image にコピー
    void copyToImage(Image& image) const {
        if (!initialized_ || !pixels_) return;

        image.allocate(width_, height_, 4);
        std::lock_guard<std::mutex> lock(mutex_);
        std::memcpy(image.getPixels(), pixels_, width_ * height_ * 4);
        image.update();
    }

    // =========================================================================
    // パーミッション（macOS）
    // =========================================================================

    // カメラ権限の状態を確認
    static bool checkCameraPermission();

    // カメラ権限をリクエスト（非同期）
    static void requestCameraPermission();

private:
    // サイズ
    int width_ = 0;
    int height_ = 0;
    int requestedWidth_ = 640;
    int requestedHeight_ = 480;
    int deviceId_ = 0;

    // 状態
    bool initialized_ = false;
    bool frameNew_ = false;

    // ピクセルデータ（RGBA）
    unsigned char* pixels_ = nullptr;

    // スレッド同期
    mutable std::mutex mutex_;
    std::atomic<bool> pixelsDirty_{false};

    // sokol リソース
    sg_image texture_ = {};
    sg_view view_ = {};
    sg_sampler sampler_ = {};
    bool textureValid_ = false;

    // プラットフォーム固有ハンドル
    void* platformHandle_ = nullptr;

    // -------------------------------------------------------------------------
    // 内部メソッド
    // -------------------------------------------------------------------------

    void createTexture() {
        if (textureValid_) {
            sg_destroy_sampler(sampler_);
            sg_destroy_view(view_);
            sg_destroy_image(texture_);
        }

        // ストリーミング用テクスチャを作成（初期データなし）
        sg_image_desc img_desc = {};
        img_desc.width = width_;
        img_desc.height = height_;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.usage.stream_update = true;  // 毎フレーム更新用
        // 注意: stream_update テクスチャは初期データを設定しない
        texture_ = sg_make_image(&img_desc);

        // ビューを作成
        sg_view_desc view_desc = {};
        view_desc.texture.image = texture_;
        view_ = sg_make_view(&view_desc);

        // サンプラーを作成（バイリニアフィルタリング）
        sg_sampler_desc smp_desc = {};
        smp_desc.min_filter = SG_FILTER_LINEAR;
        smp_desc.mag_filter = SG_FILTER_LINEAR;
        smp_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        smp_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        sampler_ = sg_make_sampler(&smp_desc);

        textureValid_ = true;
    }

    void updateTexture() {
        if (!textureValid_ || !pixels_) return;

        sg_image_data data = {};
        data.mip_levels[0].ptr = pixels_;
        data.mip_levels[0].size = width_ * height_ * 4;
        sg_update_image(texture_, &data);
    }

    void drawInternal(float x, float y, float w, float h) const {
        // アルファブレンドパイプラインを使用
        sgl_load_pipeline(internal::fontPipeline);
        sgl_enable_texture();
        sgl_texture(view_, sampler_);

        Color col = getDefaultContext().getColor();
        sgl_begin_quads();
        sgl_c4f(col.r, col.g, col.b, col.a);

        // テクスチャ座標: 左上(0,0) 右下(1,1)
        sgl_v2f_t2f(x, y, 0.0f, 0.0f);
        sgl_v2f_t2f(x + w, y, 1.0f, 0.0f);
        sgl_v2f_t2f(x + w, y + h, 1.0f, 1.0f);
        sgl_v2f_t2f(x, y + h, 0.0f, 1.0f);

        sgl_end();
        sgl_disable_texture();
        sgl_load_default_pipeline();
    }

    void moveFrom(VideoGrabber&& other) {
        width_ = other.width_;
        height_ = other.height_;
        requestedWidth_ = other.requestedWidth_;
        requestedHeight_ = other.requestedHeight_;
        deviceId_ = other.deviceId_;
        initialized_ = other.initialized_;
        frameNew_ = other.frameNew_;
        pixels_ = other.pixels_;
        pixelsDirty_.store(other.pixelsDirty_.load());
        texture_ = other.texture_;
        view_ = other.view_;
        sampler_ = other.sampler_;
        textureValid_ = other.textureValid_;
        platformHandle_ = other.platformHandle_;

        // 元のオブジェクトを無効化
        other.pixels_ = nullptr;
        other.initialized_ = false;
        other.textureValid_ = false;
        other.platformHandle_ = nullptr;
        other.width_ = 0;
        other.height_ = 0;
    }

    // -------------------------------------------------------------------------
    // リサイズ処理
    // -------------------------------------------------------------------------
    void resizeBuffers(int newWidth, int newHeight) {
        // 新しいピクセルバッファを確保
        size_t newBufferSize = newWidth * newHeight * 4;
        unsigned char* newPixels = new unsigned char[newBufferSize];
        std::memset(newPixels, 0, newBufferSize);

        // mutexでロックして古いバッファと入れ替え
        {
            std::lock_guard<std::mutex> lock(mutex_);
            delete[] pixels_;
            pixels_ = newPixels;
            width_ = newWidth;
            height_ = newHeight;
        }

        // デリゲートに新しいポインタを通知
        updateDelegatePixels();

        // テクスチャを再作成
        createTexture();
    }

    // -------------------------------------------------------------------------
    // プラットフォーム固有メソッド（tcVideoGrabber_mac.mm で実装）
    // -------------------------------------------------------------------------
    bool setupPlatform();
    void closePlatform();
    void updatePlatform();
    void updateDelegatePixels();  // ピクセルバッファ確保後にデリゲートを更新
    std::vector<VideoDeviceInfo> listDevicesPlatform();

    // リサイズ通知用（プラットフォーム実装）
    bool checkResizeNeeded();
    void getNewSize(int& width, int& height);
    void clearResizeFlag();
};

} // namespace trussc
