#pragma once

#include <cstdint>
#include <expected>
#include <string>

namespace Core {

struct RenderHostConfig {
    std::uintptr_t nativeWindowHandle = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

class RenderHost {
public:
    virtual ~RenderHost() = default;

    virtual std::expected<void, std::string> Attach(const RenderHostConfig& config) = 0;
    virtual std::expected<void, std::string> Resize(uint32_t width, uint32_t height) = 0;
    virtual std::expected<void, std::string> BeginFrame() = 0;
    virtual std::expected<void, std::string> EndFrame() = 0;
    virtual std::expected<void, std::string> Present() = 0;
    virtual void Detach() = 0;
};

} // namespace Core
