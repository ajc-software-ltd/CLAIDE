#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Core {

struct VideoFrame {
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t timestampMs = 0;
    std::vector<std::uint8_t> rgba;
};

class VideoFrameProvider {
public:
    virtual ~VideoFrameProvider() = default;

    virtual std::expected<void, std::string> Open(std::string_view filePath) = 0;
    virtual std::expected<VideoFrame, std::string> DecodeNextFrame() = 0;
    virtual void Close() = 0;
};

} // namespace Core
