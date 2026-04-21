#pragma once

#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace Core {

class AudioProvider
{
  public:
    virtual ~AudioProvider() = default;

    virtual std::expected<void, std::string> Open(std::string_view filePath) = 0;
    virtual std::expected<void, std::string> Play() = 0;
    virtual std::expected<void, std::string> Pause() = 0;
    virtual std::expected<void, std::string> Stop() = 0;
    virtual void Close() = 0;
};

} // namespace Core
