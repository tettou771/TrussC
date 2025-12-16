// =============================================================================
// tcVideoGrabber_mac.mm - macOS AVFoundation 実装
// =============================================================================

#ifdef __APPLE__

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>

#include "TrussC.h"

namespace trussc {

// ---------------------------------------------------------------------------
// プラットフォーム固有データ構造
// ---------------------------------------------------------------------------
struct VideoGrabberPlatformData {
    AVCaptureSession* session = nil;
    AVCaptureDevice* device = nil;
    AVCaptureDeviceInput* input = nil;
    AVCaptureVideoDataOutput* output = nil;
    id<AVCaptureVideoDataOutputSampleBufferDelegate> delegate = nil;
    dispatch_queue_t captureQueue = nil;

    // ダブルバッファリング用
    unsigned char* backBuffer = nullptr;
    std::atomic<bool> bufferReady{false};
    int bufferWidth = 0;
    int bufferHeight = 0;

    // サイズ変更通知用
    std::atomic<bool> needsResize{false};
    int newWidth = 0;
    int newHeight = 0;
};

} // namespace trussc

// ---------------------------------------------------------------------------
// AVCaptureVideoDataOutputSampleBufferDelegate 実装
// ---------------------------------------------------------------------------
@interface TrussCVideoGrabberDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
@property (nonatomic, assign) trussc::VideoGrabberPlatformData* platformData;
@property (nonatomic, assign) unsigned char* targetPixels;
@property (nonatomic, assign) std::atomic<bool>* pixelsDirty;
@property (nonatomic, assign) std::mutex* mutex;
@end

@implementation TrussCVideoGrabberDelegate

- (void)captureOutput:(AVCaptureOutput*)output
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection*)connection {
    if (!_platformData || !_targetPixels || !_pixelsDirty || !_mutex) return;

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!imageBuffer) return;

    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);

    void* baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);

    // サイズが変わった場合はバッファをリサイズ（最初のフレームまたはサイズ変更時）
    if (width != _platformData->bufferWidth || height != _platformData->bufferHeight) {
        NSLog(@"VideoGrabber: Frame size changed: %zu x %zu (was %d x %d)",
              width, height, _platformData->bufferWidth, _platformData->bufferHeight);

        // バックバッファをリサイズ
        if (_platformData->backBuffer) {
            delete[] _platformData->backBuffer;
        }
        _platformData->backBuffer = new unsigned char[width * height * 4];
        _platformData->bufferWidth = (int)width;
        _platformData->bufferHeight = (int)height;

        // メインスレッド側にリサイズを通知（フラグで）
        _platformData->needsResize.store(true);
        _platformData->newWidth = (int)width;
        _platformData->newHeight = (int)height;
    }

    if (baseAddress && _platformData->backBuffer) {
        unsigned char* src = (unsigned char*)baseAddress;
        unsigned char* dst = _platformData->backBuffer;

        // BGRA -> RGBA 変換してコピー
        for (size_t y = 0; y < height; y++) {
            unsigned char* srcRow = src + y * bytesPerRow;
            unsigned char* dstRow = dst + y * width * 4;
            for (size_t x = 0; x < width; x++) {
                size_t srcIdx = x * 4;
                size_t dstIdx = x * 4;
                dstRow[dstIdx + 0] = srcRow[srcIdx + 2];  // R <- B
                dstRow[dstIdx + 1] = srcRow[srcIdx + 1];  // G <- G
                dstRow[dstIdx + 2] = srcRow[srcIdx + 0];  // B <- R
                dstRow[dstIdx + 3] = 255;                  // A = 不透明
            }
        }

        // メインスレッド用バッファにコピー
        {
            std::lock_guard<std::mutex> lock(*_mutex);
            // リサイズされてない場合のみコピー（リサイズはメインスレッドで処理）
            if (!_platformData->needsResize.load()) {
                std::memcpy(_targetPixels, _platformData->backBuffer, width * height * 4);
            }
        }

        _pixelsDirty->store(true);
    }

    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
}

@end

namespace trussc {

// ---------------------------------------------------------------------------
// listDevicesPlatform - デバイス一覧を取得
// ---------------------------------------------------------------------------
std::vector<VideoDeviceInfo> VideoGrabber::listDevicesPlatform() {
    std::vector<VideoDeviceInfo> devices;

    @autoreleasepool {
        // macOS 10.15+ では AVCaptureDeviceDiscoverySession を使用
        if (@available(macOS 10.15, *)) {
            NSArray<AVCaptureDeviceType>* deviceTypes = @[
                AVCaptureDeviceTypeBuiltInWideAngleCamera,
                AVCaptureDeviceTypeExternalUnknown
            ];
            AVCaptureDeviceDiscoverySession* discoverySession =
                [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:deviceTypes
                                                                       mediaType:AVMediaTypeVideo
                                                                        position:AVCaptureDevicePositionUnspecified];
            NSArray<AVCaptureDevice*>* avDevices = discoverySession.devices;

            for (NSUInteger i = 0; i < avDevices.count; i++) {
                AVCaptureDevice* device = avDevices[i];
                VideoDeviceInfo info;
                info.deviceId = (int)i;
                info.deviceName = device.localizedName.UTF8String;
                info.uniqueId = device.uniqueID.UTF8String;
                devices.push_back(info);
            }
        } else {
            // 古い API（フォールバック）
            NSArray<AVCaptureDevice*>* avDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
            for (NSUInteger i = 0; i < avDevices.count; i++) {
                AVCaptureDevice* device = avDevices[i];
                VideoDeviceInfo info;
                info.deviceId = (int)i;
                info.deviceName = device.localizedName.UTF8String;
                info.uniqueId = device.uniqueID.UTF8String;
                devices.push_back(info);
            }
        }
    }

    return devices;
}

// ---------------------------------------------------------------------------
// setupPlatform - カメラを開始
// ---------------------------------------------------------------------------
bool VideoGrabber::setupPlatform() {
    @autoreleasepool {
        auto* data = new VideoGrabberPlatformData();
        platformHandle_ = data;

        // デバイスを取得
        NSArray<AVCaptureDevice*>* devices = nil;
        if (@available(macOS 10.15, *)) {
            NSArray<AVCaptureDeviceType>* deviceTypes = @[
                AVCaptureDeviceTypeBuiltInWideAngleCamera,
                AVCaptureDeviceTypeExternalUnknown
            ];
            AVCaptureDeviceDiscoverySession* discoverySession =
                [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:deviceTypes
                                                                       mediaType:AVMediaTypeVideo
                                                                        position:AVCaptureDevicePositionUnspecified];
            devices = discoverySession.devices;
        } else {
            devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
        }

        if (deviceId_ >= (int)devices.count) {
            NSLog(@"VideoGrabber: Invalid device ID %d (only %lu devices available)",
                  deviceId_, (unsigned long)devices.count);
            delete data;
            platformHandle_ = nullptr;
            return false;
        }

        data->device = devices[deviceId_];

        // セッションを作成
        data->session = [[AVCaptureSession alloc] init];

        // 解像度を設定（要求サイズに近いプリセットを選択）
        if (requestedWidth_ >= 1920 && requestedHeight_ >= 1080) {
            if ([data->session canSetSessionPreset:AVCaptureSessionPreset1920x1080]) {
                data->session.sessionPreset = AVCaptureSessionPreset1920x1080;
            }
        } else if (requestedWidth_ >= 1280 && requestedHeight_ >= 720) {
            if ([data->session canSetSessionPreset:AVCaptureSessionPreset1280x720]) {
                data->session.sessionPreset = AVCaptureSessionPreset1280x720;
            }
        } else if (requestedWidth_ >= 640 && requestedHeight_ >= 480) {
            if ([data->session canSetSessionPreset:AVCaptureSessionPreset640x480]) {
                data->session.sessionPreset = AVCaptureSessionPreset640x480;
            }
        } else {
            if ([data->session canSetSessionPreset:AVCaptureSessionPreset352x288]) {
                data->session.sessionPreset = AVCaptureSessionPreset352x288;
            }
        }

        // 入力を設定
        NSError* error = nil;
        data->input = [AVCaptureDeviceInput deviceInputWithDevice:data->device error:&error];
        if (error || !data->input) {
            NSLog(@"VideoGrabber: Failed to create input: %@", error.localizedDescription);
            delete data;
            platformHandle_ = nullptr;
            return false;
        }

        if (![data->session canAddInput:data->input]) {
            NSLog(@"VideoGrabber: Cannot add input to session");
            delete data;
            platformHandle_ = nullptr;
            return false;
        }
        [data->session addInput:data->input];

        // 出力を設定
        data->output = [[AVCaptureVideoDataOutput alloc] init];
        data->output.videoSettings = @{
            (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
        };
        data->output.alwaysDiscardsLateVideoFrames = YES;

        // キャプチャキューを作成
        data->captureQueue = dispatch_queue_create("trussc.videograbber", DISPATCH_QUEUE_SERIAL);

        // デリゲートを設定
        TrussCVideoGrabberDelegate* delegate = [[TrussCVideoGrabberDelegate alloc] init];
        delegate.platformData = data;
        delegate.targetPixels = pixels_;
        delegate.pixelsDirty = &pixelsDirty_;
        delegate.mutex = &mutex_;
        data->delegate = delegate;

        [data->output setSampleBufferDelegate:delegate queue:data->captureQueue];

        if (![data->session canAddOutput:data->output]) {
            NSLog(@"VideoGrabber: Cannot add output to session");
            delete data;
            platformHandle_ = nullptr;
            return false;
        }
        [data->session addOutput:data->output];

        // 実際の解像度を取得（セッション開始後に確定するので、まずプリセットから推定）
        // 後で最初のフレームで実際のサイズを確認する
        NSString* preset = data->session.sessionPreset;
        if ([preset isEqualToString:AVCaptureSessionPreset1920x1080]) {
            width_ = 1920; height_ = 1080;
        } else if ([preset isEqualToString:AVCaptureSessionPreset1280x720]) {
            width_ = 1280; height_ = 720;
        } else if ([preset isEqualToString:AVCaptureSessionPreset640x480]) {
            width_ = 640; height_ = 480;
        } else if ([preset isEqualToString:AVCaptureSessionPreset352x288]) {
            width_ = 352; height_ = 288;
        } else {
            // プリセットが不明な場合はデバイスのフォーマットから取得
            CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(data->device.activeFormat.formatDescription);
            width_ = dimensions.width;
            height_ = dimensions.height;
        }

        NSLog(@"VideoGrabber: Preset=%@, estimated size=%dx%d", preset, width_, height_);

        // バックバッファを確保
        data->bufferWidth = width_;
        data->bufferHeight = height_;
        data->backBuffer = new unsigned char[width_ * height_ * 4];
        std::memset(data->backBuffer, 0, width_ * height_ * 4);

        // キャプチャ開始
        [data->session startRunning];

        NSLog(@"VideoGrabber: Started capturing at %dx%d from %@",
              width_, height_, data->device.localizedName);

        return true;
    }
}

// ---------------------------------------------------------------------------
// closePlatform - カメラを停止
// ---------------------------------------------------------------------------
void VideoGrabber::closePlatform() {
    if (!platformHandle_) return;

    @autoreleasepool {
        auto* data = static_cast<VideoGrabberPlatformData*>(platformHandle_);

        if (data->session) {
            [data->session stopRunning];
            [data->session removeInput:data->input];
            [data->session removeOutput:data->output];
        }

        if (data->backBuffer) {
            delete[] data->backBuffer;
            data->backBuffer = nullptr;
        }

        // ARC が処理するので明示的な release は不要
        data->session = nil;
        data->device = nil;
        data->input = nil;
        data->output = nil;
        data->delegate = nil;
        data->captureQueue = nil;

        delete data;
        platformHandle_ = nullptr;
    }
}

// ---------------------------------------------------------------------------
// updatePlatform - フレーム更新（メインスレッドで呼ばれる）
// ---------------------------------------------------------------------------
void VideoGrabber::updatePlatform() {
    // AVFoundation はバックグラウンドスレッドでフレームを取得するので
    // ここでは特に何もしない（pixelsDirty_ のチェックは呼び出し元で行う）
}

// ---------------------------------------------------------------------------
// checkResizeNeeded - リサイズが必要かチェック
// ---------------------------------------------------------------------------
bool VideoGrabber::checkResizeNeeded() {
    if (!platformHandle_) return false;
    auto* data = static_cast<VideoGrabberPlatformData*>(platformHandle_);
    return data->needsResize.load();
}

// ---------------------------------------------------------------------------
// getNewSize - 新しいサイズを取得
// ---------------------------------------------------------------------------
void VideoGrabber::getNewSize(int& width, int& height) {
    if (!platformHandle_) {
        width = 0;
        height = 0;
        return;
    }
    auto* data = static_cast<VideoGrabberPlatformData*>(platformHandle_);
    width = data->newWidth;
    height = data->newHeight;
}

// ---------------------------------------------------------------------------
// clearResizeFlag - リサイズフラグをクリア
// ---------------------------------------------------------------------------
void VideoGrabber::clearResizeFlag() {
    if (!platformHandle_) return;
    auto* data = static_cast<VideoGrabberPlatformData*>(platformHandle_);
    data->needsResize.store(false);
}

// ---------------------------------------------------------------------------
// updateDelegatePixels - ピクセルバッファ確保後にデリゲートを更新
// ---------------------------------------------------------------------------
void VideoGrabber::updateDelegatePixels() {
    if (!platformHandle_) return;

    auto* data = static_cast<VideoGrabberPlatformData*>(platformHandle_);
    if (data->delegate) {
        TrussCVideoGrabberDelegate* delegate = (TrussCVideoGrabberDelegate*)data->delegate;
        delegate.targetPixels = pixels_;
        NSLog(@"VideoGrabber: Updated delegate pixels pointer to %p", pixels_);
    }
}

// ---------------------------------------------------------------------------
// checkCameraPermission - カメラ権限を確認
// ---------------------------------------------------------------------------
bool VideoGrabber::checkCameraPermission() {
    if (@available(macOS 10.14, *)) {
        AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
        return (status == AVAuthorizationStatusAuthorized);
    }
    return true;  // 10.14 より前はパーミッション不要
}

// ---------------------------------------------------------------------------
// requestCameraPermission - カメラ権限をリクエスト
// ---------------------------------------------------------------------------
void VideoGrabber::requestCameraPermission() {
    if (@available(macOS 10.14, *)) {
        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
                                 completionHandler:^(BOOL granted) {
            if (granted) {
                NSLog(@"VideoGrabber: Camera permission granted");
            } else {
                NSLog(@"VideoGrabber: Camera permission denied");
            }
        }];
    }
}

} // namespace trussc

#endif // __APPLE__
