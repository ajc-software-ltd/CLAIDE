// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        Encoding.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/Encoding.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <span>

#include <spdlog/spdlog.h>

namespace Core {

namespace {

constexpr std::array<std::uint8_t, 3> kUtf8Bom = {0xEF, 0xBB, 0xBF};
constexpr std::array<std::uint8_t, 2> kUtf16LeBom = {0xFF, 0xFE};
constexpr std::array<std::uint8_t, 2> kUtf16BeBom = {0xFE, 0xFF};

bool StartsWith(std::span<const std::uint8_t> data,
                std::span<const std::uint8_t> prefix) {
    if (data.size() < prefix.size()) {
        return false;
    }
    return std::memcmp(data.data(), prefix.data(), prefix.size()) == 0;
}

} // namespace

TextEncoding Encoding::DetectBom(std::span<const std::uint8_t> data) {
    if (StartsWith(data, kUtf8Bom)) {
        return TextEncoding::Utf8Bom;
    }
    if (StartsWith(data, kUtf16LeBom)) {
        return TextEncoding::Utf16Le;
    }
    if (StartsWith(data, kUtf16BeBom)) {
        return TextEncoding::Utf16Be;
    }
    return TextEncoding::Unknown;
}

bool Encoding::IsValidUtf8(std::string_view text) {
    std::size_t i = 0;
    while (i < text.size()) {
        std::uint32_t codePoint = 0;
        int bytes = 0;
        auto c = static_cast<std::uint8_t>(text[i]);

        if ((c & 0x80) == 0) {
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            bytes = 2;
            codePoint = c & 0x1F;
            if (c < 0xC2) {
                return false;
            }
        } else if ((c & 0xF0) == 0xE0) {
            bytes = 3;
            codePoint = c & 0x0F;
        } else if ((c & 0xF8) == 0xF0) {
            bytes = 4;
            codePoint = c & 0x07;
            if (c > 0xF4) {
                return false;
            }
        } else {
            return false;
        }

        if (i + bytes > text.size()) {
            return false;
        }

        for (int j = 1; j < bytes; ++j) {
            auto next = static_cast<std::uint8_t>(text[i + j]);
            if ((next & 0xC0) != 0x80) {
                return false;
            }

            if (j == 1) {
                if (bytes == 3) {
                    if (c == 0xE0 && next < 0xA0) {
                        return false;
                    }
                    if (c == 0xED && next > 0x9F) {
                        return false;
                    }
                } else if (bytes == 4) {
                    if (c == 0xF0 && next < 0x90) {
                        return false;
                    }
                    if (c == 0xF4 && next > 0x8F) {
                        return false;
                    }
                }
            }

            codePoint = (codePoint << 6) | (next & 0x3F);
        }

        if (codePoint > 0x10FFFF) {
            return false;
        }
        if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
            return false;
        }

        i += bytes;
    }
    return true;
}

TextEncoding Encoding::DetectEncoding(std::span<const std::uint8_t> data) {
    auto bom = DetectBom(data);
    if (bom != TextEncoding::Unknown) {
        return bom;
    }

    std::string_view text{reinterpret_cast<const char*>(data.data()),
                          data.size()};
    if (IsValidUtf8(text)) {
        bool hasHighAscii = false;
        for (auto c : data) {
            if (c > 0x7F) {
                hasHighAscii = true;
                break;
            }
        }
        return hasHighAscii ? TextEncoding::Utf8 : TextEncoding::Ascii;
    }

    return TextEncoding::Unknown;
}

std::expected<std::string, std::string> Encoding::DecodeUtf8(
    std::span<const std::uint8_t> data, bool stripBom) {
    std::size_t offset = 0;
    if (stripBom && data.size() >= kUtf8Bom.size()) {
        if (StartsWith(data, kUtf8Bom)) {
            offset = kUtf8Bom.size();
        }
    }

    std::string_view text{reinterpret_cast<const char*>(data.data()) + offset,
                          data.size() - offset};

    if (!IsValidUtf8(text)) {
        spdlog::warn("Encoding::DecodeUtf8: invalid UTF-8 sequence detected");
        return std::unexpected("Invalid UTF-8 sequence in file");
    }

    return std::string(text);
}

std::expected<std::string, std::string> Encoding::DecodeUtf16(
    std::span<const std::uint8_t> data, bool isLittleEndian) {
    if (data.size() < 2) {
        return std::string{};
    }

    std::size_t offset = 0;
    if (data.size() >= 2) {
        if ((data[0] == 0xFF && data[1] == 0xFE) ||
            (data[0] == 0xFE && data[1] == 0xFF)) {
            offset = 2;
        }
    }

    if ((data.size() - offset) % 2 != 0) {
        spdlog::warn("Encoding::DecodeUtf16: odd number of bytes after BOM");
        return std::unexpected(
            "Truncated UTF-16 data: odd number of bytes after BOM");
    }

    std::string result;
    result.reserve((data.size() - offset) / 2 * 3);

    for (std::size_t i = offset; i + 1 < data.size(); i += 2) {
        std::uint16_t codeUnit =
            isLittleEndian
                ? static_cast<std::uint16_t>(data[i]) |
                      (static_cast<std::uint16_t>(data[i + 1]) << 8)
                : static_cast<std::uint16_t>(data[i + 1]) |
                      (static_cast<std::uint16_t>(data[i]) << 8);

        if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF) {
            if (i + 3 >= data.size()) {
                return std::unexpected("Truncated UTF-16 surrogate pair");
            }
            std::uint16_t low =
                isLittleEndian
                    ? static_cast<std::uint16_t>(data[i + 2]) |
                          (static_cast<std::uint16_t>(data[i + 3]) << 8)
                    : static_cast<std::uint16_t>(data[i + 3]) |
                          (static_cast<std::uint16_t>(data[i + 2]) << 8);

            if (low < 0xDC00 || low > 0xDFFF) {
                return std::unexpected("Invalid UTF-16 low surrogate");
            }

            std::uint32_t codePoint =
                0x10000 +
                ((static_cast<std::uint32_t>(codeUnit & 0x3FF) << 10) |
                 (low & 0x3FF));

            if (codePoint <= 0xFFFF) {
                result += static_cast<char>(codePoint);
            } else if (codePoint <= 0x7FF) {
                result += static_cast<char>(0xC0 | (codePoint >> 6));
                result += static_cast<char>(0x80 | (codePoint & 0x3F));
            } else if (codePoint <= 0xFFFF) {
                result += static_cast<char>(0xE0 | (codePoint >> 12));
                result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (codePoint & 0x3F));
            } else {
                result += static_cast<char>(0xF0 | (codePoint >> 18));
                result +=
                    static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
                result +=
                    static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (codePoint & 0x3F));
            }

            i += 2;
        } else if (codeUnit >= 0xDC00 && codeUnit <= 0xDFFF) {
            return std::unexpected("Unexpected UTF-16 low surrogate");
        } else {
            if (codeUnit <= 0x7F) {
                result += static_cast<char>(codeUnit);
            } else if (codeUnit <= 0x7FF) {
                result += static_cast<char>(0xC0 | (codeUnit >> 6));
                result += static_cast<char>(0x80 | (codeUnit & 0x3F));
            } else {
                result += static_cast<char>(0xE0 | (codeUnit >> 12));
                result +=
                    static_cast<char>(0x80 | ((codeUnit >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (codeUnit & 0x3F));
            }
        }
    }

    return result;
}

std::expected<DecodeResult, std::string> Encoding::Decode(
    std::span<const std::uint8_t> data) {
    if (data.empty()) {
        return DecodeResult{"", TextEncoding::Ascii, false, std::nullopt};
    }

    auto detected = DetectBom(data);

    if (detected == TextEncoding::Utf16Le) {
        auto text = DecodeUtf16(data, true);
        if (!text) {
            return std::unexpected(text.error());
        }
        spdlog::debug("Encoding::Decode: detected UTF-16 LE with BOM");
        return DecodeResult{*std::move(text), TextEncoding::Utf16Le, true,
                            std::nullopt};
    }

    if (detected == TextEncoding::Utf16Be) {
        auto text = DecodeUtf16(data, false);
        if (!text) {
            return std::unexpected(text.error());
        }
        spdlog::debug("Encoding::Decode: detected UTF-16 BE with BOM");
        return DecodeResult{*std::move(text), TextEncoding::Utf16Be, true,
                            std::nullopt};
    }

    if (detected == TextEncoding::Utf8Bom) {
        auto text = DecodeUtf8(data, true);
        if (!text) {
            return std::unexpected(text.error());
        }
        spdlog::debug("Encoding::Decode: detected UTF-8 with BOM");
        return DecodeResult{*std::move(text), TextEncoding::Utf8Bom, true,
                            std::nullopt};
    }

    auto encoding = DetectEncoding(data);
    if (encoding == TextEncoding::Unknown) {
        spdlog::warn("Encoding::Decode: unable to detect encoding, treating as "
                     "UTF-8");
        return std::unexpected(
            "Unable to reliably detect file encoding. Treating as UTF-8.");
    }

    auto text = DecodeUtf8(data, false);
    if (!text) {
        return std::unexpected("File contains invalid UTF-8 data");
    }

    spdlog::debug("Encoding::Decode: detected {}",
                  EncodingName(encoding));
    return DecodeResult{*std::move(text), encoding, false, std::nullopt};
}

std::expected<EncodeResult, std::string> Encoding::EncodeUtf16(
    std::string_view text, bool isLittleEndian) {
    std::vector<std::uint8_t> result;
    result.reserve(text.size() * 2 + 2);

    if (isLittleEndian) {
        result.push_back(0xFF);
        result.push_back(0xFE);
    } else {
        result.push_back(0xFE);
        result.push_back(0xFF);
    }

    std::size_t i = 0;
    while (i < text.size()) {
        std::uint32_t codePoint = 0;
        auto c = static_cast<std::uint8_t>(text[i]);

        if ((c & 0x80) == 0) {
            codePoint = c;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= text.size()) {
                return std::unexpected("Truncated UTF-8 sequence");
            }
            codePoint = (static_cast<std::uint32_t>(c & 0x1F) << 6) |
                        (static_cast<std::uint8_t>(text[i + 1]) & 0x3F);
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= text.size()) {
                return std::unexpected("Truncated UTF-8 sequence");
            }
            codePoint = (static_cast<std::uint32_t>(c & 0x0F) << 12) |
                        ((static_cast<std::uint8_t>(text[i + 1]) & 0x3F) << 6) |
                        (static_cast<std::uint8_t>(text[i + 2]) & 0x3F);
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= text.size()) {
                return std::unexpected("Truncated UTF-8 sequence");
            }
            codePoint = (static_cast<std::uint32_t>(c & 0x07) << 18) |
                        ((static_cast<std::uint8_t>(text[i + 1]) & 0x3F) << 12) |
                        ((static_cast<std::uint8_t>(text[i + 2]) & 0x3F) << 6) |
                        (static_cast<std::uint8_t>(text[i + 3]) & 0x3F);
            i += 4;
        } else {
            return std::unexpected("Invalid UTF-8 byte");
        }

        if (codePoint <= 0xFFFF) {
            if (isLittleEndian) {
                result.push_back(static_cast<std::uint8_t>(codePoint & 0xFF));
                result.push_back(
                    static_cast<std::uint8_t>((codePoint >> 8) & 0xFF));
            } else {
                result.push_back(
                    static_cast<std::uint8_t>((codePoint >> 8) & 0xFF));
                result.push_back(static_cast<std::uint8_t>(codePoint & 0xFF));
            }
        } else {
            codePoint -= 0x10000;
            std::uint16_t high = 0xD800 | ((codePoint >> 10) & 0x3FF);
            std::uint16_t low = 0xDC00 | (codePoint & 0x3FF);

            if (isLittleEndian) {
                result.push_back(static_cast<std::uint8_t>(high & 0xFF));
                result.push_back(
                    static_cast<std::uint8_t>((high >> 8) & 0xFF));
                result.push_back(static_cast<std::uint8_t>(low & 0xFF));
                result.push_back(static_cast<std::uint8_t>((low >> 8) & 0xFF));
            } else {
                result.push_back(
                    static_cast<std::uint8_t>((high >> 8) & 0xFF));
                result.push_back(static_cast<std::uint8_t>(high & 0xFF));
                result.push_back(
                    static_cast<std::uint8_t>((low >> 8) & 0xFF));
                result.push_back(static_cast<std::uint8_t>(low & 0xFF));
            }
        }
    }

    return EncodeResult{std::move(result)};
}

std::expected<EncodeResult, std::string> Encoding::Encode(
    std::string_view text, TextEncoding encoding) {
    switch (encoding) {
    case TextEncoding::Utf8Bom: {
        std::vector<std::uint8_t> result;
        result.reserve(text.size() + kUtf8Bom.size());
        result.insert(result.end(), kUtf8Bom.begin(), kUtf8Bom.end());
        result.insert(result.end(), text.begin(), text.end());
        return EncodeResult{std::move(result)};
    }
    case TextEncoding::Utf16Le: {
        return EncodeUtf16(text, true);
    }
    case TextEncoding::Utf16Be: {
        return EncodeUtf16(text, false);
    }
    case TextEncoding::Utf8:
    case TextEncoding::Ascii:
    default: {
        std::vector<std::uint8_t> result{text.begin(), text.end()};
        return EncodeResult{std::move(result)};
    }
    }
}

std::string_view Encoding::EncodingName(TextEncoding encoding) {
    switch (encoding) {
    case TextEncoding::Utf8:
        return "UTF-8";
    case TextEncoding::Utf8Bom:
        return "UTF-8 (BOM)";
    case TextEncoding::Utf16Le:
        return "UTF-16 LE";
    case TextEncoding::Utf16Be:
        return "UTF-16 BE";
    case TextEncoding::Ascii:
        return "ASCII";
    case TextEncoding::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

bool Encoding::HasBom(TextEncoding encoding) {
    return encoding == TextEncoding::Utf8Bom ||
           encoding == TextEncoding::Utf16Le ||
           encoding == TextEncoding::Utf16Be;
}

} // namespace Core
