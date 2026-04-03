// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        Encoding.hpp
// Project:     CLIADE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Core {

enum class TextEncoding {
    Utf8,
    Utf8Bom,
    Utf16Le,
    Utf16Be,
    Ascii,
    Unknown
};

struct DecodeResult {
    std::string text;
    TextEncoding detectedEncoding;
    bool hadBom;
    std::optional<std::string> warning;
};

struct EncodeResult {
    std::vector<std::uint8_t> bytes;
};

class Encoding {
public:
    static TextEncoding DetectEncoding(std::span<const std::uint8_t> data);

    static std::expected<DecodeResult, std::string> Decode(
        std::span<const std::uint8_t> data);

    static std::expected<EncodeResult, std::string> Encode(
        std::string_view text, TextEncoding encoding);

    static std::string_view EncodingName(TextEncoding encoding);

    static bool HasBom(TextEncoding encoding);

private:
    static TextEncoding DetectBom(std::span<const std::uint8_t> data);
    static bool IsValidUtf8(std::string_view text);
    static std::expected<std::string, std::string> DecodeUtf8(
        std::span<const std::uint8_t> data, bool stripBom);
    static std::expected<std::string, std::string> DecodeUtf16(
        std::span<const std::uint8_t> data, bool isLittleEndian);
    static std::expected<EncodeResult, std::string> EncodeUtf16(
        std::string_view text, bool isLittleEndian);
};

} // namespace Core
