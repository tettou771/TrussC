#pragma once

// =============================================================================
// tcTcvPlayer.h - TCVC video player (v2: I/P/REF frame support)
// =============================================================================

#include <TrussC.h>
#include <fstream>
#include <vector>
#include <cstring>
#include <unordered_map>

#include "tcTcvEncoder.h"  // For TcvHeader and constants
#include "impl/bcdec.h"    // For BC7 decompression

namespace tcx {

// ---------------------------------------------------------------------------
// TcvPlayer - Plays TCVC encoded video
// ---------------------------------------------------------------------------
class TcvPlayer : public tc::VideoPlayerBase {
public:
    // Debug block types for visualization
    enum class DebugBlockType : uint8_t {
        None,    // I-frame or no debug
        Skip,    // Copy from reference
        Solid,   // Solid color
        QuarterBC7,  // Quarter BC7
        BC7      // Full BC7
    };

    TcvPlayer() {
        bc7enc_compress_block_init();
    }
    ~TcvPlayer() { close(); }

    // Non-copyable
    TcvPlayer(const TcvPlayer&) = delete;
    TcvPlayer& operator=(const TcvPlayer&) = delete;

    // Debug mode
    void setDebug(bool enabled) { debugMode_ = enabled; }
    bool isDebug() const { return debugMode_; }

    // Draw debug overlay (call after drawing video)
    void drawDebugOverlay(float x, float y, float scale = 1.0f) {
        if (!debugMode_ || debugBlockTypes_.empty()) return;

        float blockSize = TCV_BLOCK_SIZE * scale;

        tc::noFill();

        for (int by = 0; by < blocksY_; by++) {
            for (int bx = 0; bx < blocksX_; bx++) {
                int idx = by * blocksX_ + bx;
                DebugBlockType type = debugBlockTypes_[idx];

                // Skip None and Skip types (Skip is obvious from context)
                if (type == DebugBlockType::None || type == DebugBlockType::Skip) continue;

                float rx = x + bx * blockSize;
                float ry = y + by * blockSize;

                // Set color based on block type
                switch (type) {
                    case DebugBlockType::Solid:
                        tc::setColor(0.0f, 1.0f, 0.0f, 0.8f);  // Green for solid
                        break;
                    case DebugBlockType::QuarterBC7:
                        tc::setColor(1.0f, 1.0f, 0.0f, 0.8f);  // Yellow for Q-BC7
                        break;
                    case DebugBlockType::BC7:
                        tc::setColor(1.0f, 0.0f, 0.0f, 0.8f);  // Red for BC7
                        break;
                    default:
                        continue;
                }

                // Draw 1px rectangle outline
                tc::drawRect(rx + 0.5f, ry + 0.5f, blockSize - 1, blockSize - 1);
            }
        }

        // Reset drawing state
        tc::fill();
        tc::setColor(1.0f);
    }

    // =========================================================================
    // Load / Close
    // =========================================================================

    bool load(const std::string& path) override {
        if (initialized_) {
            close();
        }

        file_.open(path, std::ios::binary);
        if (!file_.is_open()) {
            tc::logError("TcvPlayer") << "Failed to open file: " << path;
            return false;
        }

        // Read header
        file_.read(reinterpret_cast<char*>(&header_), sizeof(header_));

        // Validate signature
        if (std::memcmp(header_.signature, "TCVC", 4) != 0) {
            tc::logError("TcvPlayer") << "Invalid TCVC signature";
            file_.close();
            return false;
        }

        // Validate version
        if (header_.version != TCV_VERSION) {
            tc::logError("TcvPlayer") << "Unsupported version: " << header_.version
                                      << " (expected " << TCV_VERSION << ")";
            file_.close();
            return false;
        }

        width_ = header_.width;
        height_ = header_.height;

        // Calculate block counts
        blocksX_ = (width_ + TCV_BLOCK_SIZE - 1) / TCV_BLOCK_SIZE;
        blocksY_ = (height_ + TCV_BLOCK_SIZE - 1) / TCV_BLOCK_SIZE;
        totalBlocks_ = blocksX_ * blocksY_;

        // Calculate BC7 data size per frame
        bc7FrameSize_ = totalBlocks_ * 256;  // 16 BC7 blocks per 16x16, 16 bytes each

        // Allocate BC7 buffer
        bc7Buffer_.resize(bc7FrameSize_);

        // Allocate debug block types buffer
        debugBlockTypes_.resize(totalBlocks_, DebugBlockType::None);

        // Build frame index for seeking
        buildFrameIndex();

        // Allocate texture as BC7 compressed
        int paddedWidth = blocksX_ * TCV_BLOCK_SIZE;
        int paddedHeight = blocksY_ * TCV_BLOCK_SIZE;
        texture_.allocateCompressed(paddedWidth, paddedHeight,
                                    SG_PIXELFORMAT_BC7_RGBA,
                                    nullptr, 0);

        initialized_ = true;
        currentFrame_ = -1;

        tc::logNotice("TcvPlayer") << "Loaded: " << width_ << "x" << height_
                                   << " @ " << header_.fps << " fps, "
                                   << header_.frameCount << " frames";
        return true;
    }

    void close() override {
        if (!initialized_) return;

        file_.close();
        texture_.clear();
        frameIndex_.clear();
        iFrameCache_.clear();
        bc7Buffer_.clear();
        debugBlockTypes_.clear();

        initialized_ = false;
        playing_ = false;
        paused_ = false;
        frameNew_ = false;
        firstFrameReceived_ = false;
        done_ = false;
        currentFrame_ = -1;
    }

    // =========================================================================
    // Update
    // =========================================================================

    void update() override {
        if (!initialized_ || !playing_ || paused_) return;

        frameNew_ = false;

        // Calculate target frame based on elapsed time
        float elapsed = tc::getElapsedTimef() - playStartTime_;
        int targetFrame = static_cast<int>(elapsed * header_.fps);

        if (targetFrame >= static_cast<int>(header_.frameCount)) {
            if (loop_) {
                playStartTime_ = tc::getElapsedTimef();
                targetFrame = 0;
            } else {
                markDone();
                return;
            }
        }

        if (targetFrame != currentFrame_) {
            decodeFrame(targetFrame);
            currentFrame_ = targetFrame;
            markFrameNew();
        }
    }

    // =========================================================================
    // Properties
    // =========================================================================

    float getDuration() const override {
        if (!initialized_ || header_.fps <= 0) return 0.0f;
        return static_cast<float>(header_.frameCount) / header_.fps;
    }

    float getPosition() const override {
        if (!initialized_ || header_.frameCount == 0) return 0.0f;
        return static_cast<float>(currentFrame_) / header_.frameCount;
    }

    // =========================================================================
    // Frame control
    // =========================================================================

    int getCurrentFrame() const override {
        return currentFrame_ >= 0 ? currentFrame_ : 0;
    }

    int getTotalFrames() const override {
        return initialized_ ? header_.frameCount : 0;
    }

    void setFrame(int frame) override {
        if (!initialized_) return;
        frame = std::max(0, std::min(frame, static_cast<int>(header_.frameCount) - 1));
        if (frame != currentFrame_) {
            decodeFrame(frame);
            currentFrame_ = frame;
            markFrameNew();
        }
    }

    void nextFrame() override {
        setFrame(currentFrame_ + 1);
    }

    void previousFrame() override {
        setFrame(currentFrame_ - 1);
    }

protected:
    void playImpl() override {
        playStartTime_ = tc::getElapsedTimef();
        currentFrame_ = -1;
    }

    void stopImpl() override {
        currentFrame_ = -1;
    }

    void setPausedImpl(bool paused) override {
        // TODO: Handle pause timer properly
    }

    void setPositionImpl(float pct) override {
        int frame = static_cast<int>(pct * header_.frameCount);
        setFrame(frame);
        playStartTime_ = tc::getElapsedTimef() - pct * getDuration();
    }

    void setVolumeImpl(float vol) override {
        // TODO: Audio support in Phase 4
    }

    void setSpeedImpl(float speed) override {
        // TODO: Playback speed support
    }

    void setLoopImpl(bool loop) override {
        // Already handled by base class
    }

private:
    std::ifstream file_;
    TcvHeader header_ = {};

    int blocksX_ = 0;
    int blocksY_ = 0;
    int totalBlocks_ = 0;
    size_t bc7FrameSize_ = 0;

    std::vector<uint8_t> bc7Buffer_;
    int currentFrame_ = -1;

    float playStartTime_ = 0.0f;

    // Frame index entry
    struct FrameIndexEntry {
        std::streampos offset;
        uint8_t packetType;
        uint32_t refFrame;      // For P/REF frames
        uint32_t dataSize;
    };
    std::vector<FrameIndexEntry> frameIndex_;

    // I-frame cache: frameNumber -> BC7 data
    std::unordered_map<int, std::vector<uint8_t>> iFrameCache_;

    // Debug mode
    bool debugMode_ = false;
    std::vector<DebugBlockType> debugBlockTypes_;

    // Build index of frame offsets for seeking
    void buildFrameIndex() {
        frameIndex_.clear();
        frameIndex_.reserve(header_.frameCount);
        iFrameCache_.clear();

        file_.seekg(header_.headerSize);

        for (uint32_t i = 0; i < header_.frameCount; i++) {
            FrameIndexEntry entry;
            entry.offset = file_.tellg();

            file_.read(reinterpret_cast<char*>(&entry.packetType), 1);

            if (entry.packetType == TCV_PACKET_I_FRAME) {
                file_.read(reinterpret_cast<char*>(&entry.dataSize), 4);
                entry.refFrame = 0;
                file_.seekg(entry.dataSize, std::ios::cur);
            } else if (entry.packetType == TCV_PACKET_P_FRAME) {
                file_.read(reinterpret_cast<char*>(&entry.refFrame), 4);
                file_.read(reinterpret_cast<char*>(&entry.dataSize), 4);
                file_.seekg(entry.dataSize, std::ios::cur);
            } else if (entry.packetType == TCV_PACKET_REF_FRAME) {
                file_.read(reinterpret_cast<char*>(&entry.refFrame), 4);
                entry.dataSize = 0;
            } else {
                tc::logWarning("TcvPlayer") << "Unknown packet type at frame " << i;
                break;
            }

            frameIndex_.push_back(entry);
        }

        tc::logNotice("TcvPlayer") << "Indexed " << frameIndex_.size() << " frames";
    }

    // Get or decode I-frame BC7 data (returns GPU layout BC7 buffer)
    const std::vector<uint8_t>& getIFrameData(int frameNum) {
        // Check cache
        auto it = iFrameCache_.find(frameNum);
        if (it != iFrameCache_.end()) {
            return it->second;
        }

        // Load from file
        const auto& entry = frameIndex_[frameNum];
        if (entry.packetType != TCV_PACKET_I_FRAME) {
            tc::logError("TcvPlayer") << "Frame " << frameNum << " is not an I-frame";
            static std::vector<uint8_t> empty;
            return empty;
        }

        // Allocate result buffer (full GPU layout BC7 data)
        std::vector<uint8_t> bc7Result(bc7FrameSize_);

        // Seek to frame data
        file_.seekg(entry.offset);
        uint8_t packetType;
        uint32_t dataSize;
        file_.read(reinterpret_cast<char*>(&packetType), 1);
        file_.read(reinterpret_cast<char*>(&dataSize), 4);

        // Parse block commands (same format as P-frame but no SKIP)
        int blockIdx = 0;
        while (blockIdx < totalBlocks_) {
            uint8_t cmd;
            file_.read(reinterpret_cast<char*>(&cmd), 1);

            uint8_t blockType = cmd & TCV_BLOCK_TYPE_MASK;
            int runLength = (cmd & TCV_BLOCK_RUN_MASK) + 1;

            // Read SOLID color once for entire run
            uint32_t solidColor = 0;
            if (blockType == TCV_BLOCK_SOLID) {
                file_.read(reinterpret_cast<char*>(&solidColor), 4);
            }

            for (int i = 0; i < runLength && blockIdx < totalBlocks_; i++, blockIdx++) {
                int bx16 = blockIdx % blocksX_;
                int by16 = blockIdx / blocksX_;

                if (blockType == TCV_BLOCK_SOLID) {
                    writeIFrameSolidBlock(bc7Result.data(), bx16, by16, solidColor);
                } else if (blockType == TCV_BLOCK_BC7_Q) {
                    uint8_t bc7Block[16];
                    file_.read(reinterpret_cast<char*>(bc7Block), 16);
                    writeIFrameQuarterBC7(bc7Result.data(), bx16, by16, bc7Block);
                } else if (blockType == TCV_BLOCK_BC7) {
                    uint8_t blockData[256];
                    file_.read(reinterpret_cast<char*>(blockData), 256);
                    writeIFrameBC7Block(bc7Result.data(), bx16, by16, blockData);
                }
            }
        }

        // Cache it (limit cache size to avoid memory issues)
        if (iFrameCache_.size() >= TCV_IFRAME_BUFFER_SIZE) {
            iFrameCache_.erase(iFrameCache_.begin());
        }
        iFrameCache_[frameNum] = std::move(bc7Result);

        return iFrameCache_[frameNum];
    }

    // Decode I-frame with block commands (for display + debug)
    void decodeIFrameWithBlocks(int frameNum, const FrameIndexEntry& entry) {
        // Check if already cached
        auto it = iFrameCache_.find(frameNum);
        if (it != iFrameCache_.end()) {
            // Use cached data
            std::memcpy(bc7Buffer_.data(), it->second.data(), bc7FrameSize_);
            // Still need to parse for debug info
            if (!debugMode_) return;
        }

        // Need to decode from file
        file_.seekg(entry.offset);
        uint8_t packetType;
        uint32_t dataSize;
        file_.read(reinterpret_cast<char*>(&packetType), 1);
        file_.read(reinterpret_cast<char*>(&dataSize), 4);

        // If not cached, we'll build the BC7 buffer while parsing
        bool needBuild = (it == iFrameCache_.end());

        int blockIdx = 0;
        while (blockIdx < totalBlocks_) {
            uint8_t cmd;
            file_.read(reinterpret_cast<char*>(&cmd), 1);

            uint8_t blockType = cmd & TCV_BLOCK_TYPE_MASK;
            int runLength = (cmd & TCV_BLOCK_RUN_MASK) + 1;

            // Read SOLID color once for entire run
            uint32_t solidColor = 0;
            if (blockType == TCV_BLOCK_SOLID) {
                file_.read(reinterpret_cast<char*>(&solidColor), 4);
            }

            for (int i = 0; i < runLength && blockIdx < totalBlocks_; i++, blockIdx++) {
                int bx16 = blockIdx % blocksX_;
                int by16 = blockIdx / blocksX_;

                if (blockType == TCV_BLOCK_SOLID) {
                    if (needBuild) writeSolidBlockToGpuLayout(bx16, by16, solidColor);
                    if (debugMode_) debugBlockTypes_[blockIdx] = DebugBlockType::Solid;
                } else if (blockType == TCV_BLOCK_BC7_Q) {
                    uint8_t bc7Block[16];
                    file_.read(reinterpret_cast<char*>(bc7Block), 16);
                    if (needBuild) writeQuarterBC7ToGpuLayout(bx16, by16, bc7Block);
                    if (debugMode_) debugBlockTypes_[blockIdx] = DebugBlockType::QuarterBC7;
                } else if (blockType == TCV_BLOCK_BC7) {
                    uint8_t blockData[256];
                    file_.read(reinterpret_cast<char*>(blockData), 256);
                    if (needBuild) writeBlockToGpuLayout(bx16, by16, blockData);
                    if (debugMode_) debugBlockTypes_[blockIdx] = DebugBlockType::BC7;
                }
            }
        }

        // Cache the result if not already cached
        if (needBuild) {
            if (iFrameCache_.size() >= TCV_IFRAME_BUFFER_SIZE) {
                iFrameCache_.erase(iFrameCache_.begin());
            }
            iFrameCache_[frameNum] = std::vector<uint8_t>(bc7Buffer_.begin(), bc7Buffer_.end());
        }
    }

    // Helper functions for I-frame decoding (write to arbitrary buffer)
    void writeIFrameBC7Block(uint8_t* buffer, int bx16, int by16, const uint8_t* bc7Data) {
        int bc7BlocksX = blocksX_ * 4;
        for (int by4 = 0; by4 < 4; by4++) {
            for (int bx4 = 0; bx4 < 4; bx4++) {
                int gpuX = bx16 * 4 + bx4;
                int gpuY = by16 * 4 + by4;
                int gpuIdx = gpuY * bc7BlocksX + gpuX;
                std::memcpy(buffer + gpuIdx * 16, bc7Data + (by4 * 4 + bx4) * 16, 16);
            }
        }
    }

    void writeIFrameSolidBlock(uint8_t* buffer, int bx16, int by16, uint32_t color) {
        uint8_t block4x4[64];
        for (int i = 0; i < 16; i++) {
            std::memcpy(block4x4 + i * 4, &color, 4);
        }

        bc7enc_compress_block_params params;
        bc7enc_compress_block_params_init(&params);
        params.m_max_partitions = 0;
        params.m_uber_level = 0;

        int bc7BlocksX = blocksX_ * 4;
        for (int by4 = 0; by4 < 4; by4++) {
            for (int bx4 = 0; bx4 < 4; bx4++) {
                int gpuX = bx16 * 4 + bx4;
                int gpuY = by16 * 4 + by4;
                int gpuIdx = gpuY * bc7BlocksX + gpuX;
                bc7enc_compress_block(buffer + gpuIdx * 16, block4x4, &params);
            }
        }
    }

    void writeIFrameQuarterBC7(uint8_t* buffer, int bx16, int by16, const uint8_t* bc7Data) {
        uint8_t decoded4x4[64];
        bcdec_bc7(bc7Data, decoded4x4, 4 * 4);

        bc7enc_compress_block_params params;
        bc7enc_compress_block_params_init(&params);
        params.m_max_partitions = 0;
        params.m_uber_level = 0;

        int bc7BlocksX = blocksX_ * 4;
        for (int dy = 0; dy < 4; dy++) {
            for (int dx = 0; dx < 4; dx++) {
                const uint8_t* pixel = decoded4x4 + (dy * 4 + dx) * 4;
                uint8_t block4x4[64];
                for (int i = 0; i < 16; i++) {
                    std::memcpy(block4x4 + i * 4, pixel, 4);
                }

                int gpuX = bx16 * 4 + dx;
                int gpuY = by16 * 4 + dy;
                int gpuIdx = gpuY * bc7BlocksX + gpuX;
                bc7enc_compress_block(buffer + gpuIdx * 16, block4x4, &params);
            }
        }
    }

    // Write a 16x16 block (16 4x4 blocks) to GPU layout buffer
    // bx16, by16: 16x16 block coordinates
    void writeBlockToGpuLayout(int bx16, int by16, const uint8_t* bc7Data) {
        int bc7BlocksX = blocksX_ * 4;  // 4x4 blocks per row

        for (int by4 = 0; by4 < 4; by4++) {
            for (int bx4 = 0; bx4 < 4; bx4++) {
                // GPU coordinates for this 4x4 block
                int gpuX = bx16 * 4 + bx4;
                int gpuY = by16 * 4 + by4;
                int gpuIdx = gpuY * bc7BlocksX + gpuX;

                // Copy 16 bytes (one BC7 block)
                std::memcpy(bc7Buffer_.data() + gpuIdx * 16,
                           bc7Data + (by4 * 4 + bx4) * 16,
                           16);
            }
        }
    }

    // Upscale quarter BC7 (4x4 BC7 block) to 16x16 and write to GPU layout
    void writeQuarterBC7ToGpuLayout(int bx16, int by16, const uint8_t* bc7Data) {
        // Decode the single BC7 block to 4x4 pixels (pitch = 4 pixels * 4 bytes)
        uint8_t decoded4x4[64];
        bcdec_bc7(bc7Data, decoded4x4, 4 * 4);

        bc7enc_compress_block_params params;
        bc7enc_compress_block_params_init(&params);
        params.m_max_partitions = 0;  // Fast for solid blocks
        params.m_uber_level = 0;

        int bc7BlocksX = blocksX_ * 4;

        // Each pixel in 4x4 becomes a 4x4 region of solid color
        for (int dy = 0; dy < 4; dy++) {
            for (int dx = 0; dx < 4; dx++) {
                // Get the color from decoded pixel
                const uint8_t* pixel = decoded4x4 + (dy * 4 + dx) * 4;

                // Create solid 4x4 block
                uint8_t block4x4[64];
                for (int i = 0; i < 16; i++) {
                    std::memcpy(block4x4 + i * 4, pixel, 4);
                }

                // Encode and write to GPU position
                int gpuX = bx16 * 4 + dx;
                int gpuY = by16 * 4 + dy;
                int gpuIdx = gpuY * bc7BlocksX + gpuX;

                bc7enc_compress_block(bc7Buffer_.data() + gpuIdx * 16, block4x4, &params);
            }
        }
    }

    // Encode solid color to BC7 and write to GPU layout
    void writeSolidBlockToGpuLayout(int bx16, int by16, uint32_t color) {
        // Create a 4x4 block of solid color
        uint8_t block4x4[64];
        for (int i = 0; i < 16; i++) {
            std::memcpy(block4x4 + i * 4, &color, 4);
        }

        bc7enc_compress_block_params params;
        bc7enc_compress_block_params_init(&params);
        params.m_max_partitions = 0;
        params.m_uber_level = 0;

        int bc7BlocksX = blocksX_ * 4;

        // Encode and write each 4x4 block to correct GPU position
        for (int by4 = 0; by4 < 4; by4++) {
            for (int bx4 = 0; bx4 < 4; bx4++) {
                int gpuX = bx16 * 4 + bx4;
                int gpuY = by16 * 4 + by4;
                int gpuIdx = gpuY * bc7BlocksX + gpuX;

                bc7enc_compress_block(bc7Buffer_.data() + gpuIdx * 16, block4x4, &params);
            }
        }
    }

    // Decode a specific frame
    void decodeFrame(int frameNum) {
        if (frameNum < 0 || frameNum >= static_cast<int>(frameIndex_.size())) {
            return;
        }

        const auto& entry = frameIndex_[frameNum];

        // Clear debug block types (None = no overlay for I-frames)
        if (debugMode_) {
            std::fill(debugBlockTypes_.begin(), debugBlockTypes_.end(), DebugBlockType::None);
        }

        if (entry.packetType == TCV_PACKET_I_FRAME) {
            // Decode I-frame with block commands (same format as P-frame but no SKIP)
            decodeIFrameWithBlocks(frameNum, entry);
        } else if (entry.packetType == TCV_PACKET_REF_FRAME) {
            // REF-frame: copy from referenced I-frame
            const auto& refData = getIFrameData(entry.refFrame);
            if (refData.size() == bc7FrameSize_) {
                std::memcpy(bc7Buffer_.data(), refData.data(), bc7FrameSize_);
            }
            // For debug: REF-frame = all Skip (copy from I-frame)
            if (debugMode_) {
                std::fill(debugBlockTypes_.begin(), debugBlockTypes_.end(), DebugBlockType::Skip);
            }
        } else if (entry.packetType == TCV_PACKET_P_FRAME) {
            // P-frame: start with reference (GPU layout), apply deltas
            const auto& refData = getIFrameData(entry.refFrame);
            if (refData.size() == bc7FrameSize_) {
                std::memcpy(bc7Buffer_.data(), refData.data(), bc7FrameSize_);
            }

            // Read and apply block commands
            file_.seekg(entry.offset);
            uint8_t packetType;
            uint32_t refFrame, dataSize;
            file_.read(reinterpret_cast<char*>(&packetType), 1);
            file_.read(reinterpret_cast<char*>(&refFrame), 4);
            file_.read(reinterpret_cast<char*>(&dataSize), 4);

            int blockIdx = 0;
            while (blockIdx < totalBlocks_) {
                uint8_t cmd;
                file_.read(reinterpret_cast<char*>(&cmd), 1);

                uint8_t blockType = cmd & TCV_BLOCK_TYPE_MASK;
                int runLength = (cmd & TCV_BLOCK_RUN_MASK) + 1;

                // Read SOLID color once for entire run
                uint32_t solidColor = 0;
                if (blockType == TCV_BLOCK_SOLID) {
                    file_.read(reinterpret_cast<char*>(&solidColor), 4);
                }

                for (int i = 0; i < runLength && blockIdx < totalBlocks_; i++, blockIdx++) {
                    int bx16 = blockIdx % blocksX_;
                    int by16 = blockIdx / blocksX_;

                    if (blockType == TCV_BLOCK_SKIP) {
                        // Keep reference data (already copied in GPU layout)
                        if (debugMode_) debugBlockTypes_[blockIdx] = DebugBlockType::Skip;
                    } else if (blockType == TCV_BLOCK_SOLID) {
                        writeSolidBlockToGpuLayout(bx16, by16, solidColor);
                        if (debugMode_) debugBlockTypes_[blockIdx] = DebugBlockType::Solid;
                    } else if (blockType == TCV_BLOCK_BC7_Q) {
                        // Read quarter BC7 (single 4x4 BC7 block = 16 bytes)
                        uint8_t bc7Block[16];
                        file_.read(reinterpret_cast<char*>(bc7Block), 16);
                        writeQuarterBC7ToGpuLayout(bx16, by16, bc7Block);
                        if (debugMode_) debugBlockTypes_[blockIdx] = DebugBlockType::QuarterBC7;
                    } else if (blockType == TCV_BLOCK_BC7) {
                        // Read 16x16 block data (256 bytes = 16 BC7 4x4 blocks)
                        uint8_t blockData[256];
                        file_.read(reinterpret_cast<char*>(blockData), 256);
                        writeBlockToGpuLayout(bx16, by16, blockData);
                        if (debugMode_) debugBlockTypes_[blockIdx] = DebugBlockType::BC7;
                    }
                }
            }
        }

        // Upload to GPU texture
        texture_.updateCompressed(bc7Buffer_.data(), bc7FrameSize_);
    }
};

} // namespace tcx
