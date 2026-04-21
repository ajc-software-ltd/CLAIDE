// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        EncodingTests.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <span>
#include <vector>

#include "core/Encoding.hpp"

using namespace Core;

TEST_CASE("DetectEncoding detects UTF-8 BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {0xEF, 0xBB, 0xBF, 'H', 'i'};
    auto result = Encoding::DetectEncoding(data);
    REQUIRE(result == TextEncoding::Utf8Bom);
}

TEST_CASE("DetectEncoding detects UTF-16 LE BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {0xFF, 0xFE, 'H', 0x00};
    auto result = Encoding::DetectEncoding(data);
    REQUIRE(result == TextEncoding::Utf16Le);
}

TEST_CASE("DetectEncoding detects UTF-16 BE BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {0xFE, 0xFF, 0x00, 'H'};
    auto result = Encoding::DetectEncoding(data);
    REQUIRE(result == TextEncoding::Utf16Be);
}

TEST_CASE("DetectEncoding detects plain ASCII", "[encoding]") {
    std::vector<std::uint8_t> data = {'H', 'e', 'l', 'l', 'o'};
    auto result = Encoding::DetectEncoding(data);
    REQUIRE(result == TextEncoding::Ascii);
}

TEST_CASE("DetectEncoding detects UTF-8 without BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {0xC3, 0xA9, 0xC3, 0xA0};
    auto result = Encoding::DetectEncoding(data);
    REQUIRE(result == TextEncoding::Utf8);
}

TEST_CASE("Decode decodes UTF-8 without BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {'H', 'e', 'l', 'l', 'o'};
    auto result = Encoding::Decode(data);
    REQUIRE(result.has_value());
    REQUIRE(result->text == "Hello");
    REQUIRE(result->detectedEncoding == TextEncoding::Ascii);
    REQUIRE(!result->hadBom);
}

TEST_CASE("Decode decodes UTF-8 with BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {0xEF, 0xBB, 0xBF, 'H', 'i'};
    auto result = Encoding::Decode(data);
    REQUIRE(result.has_value());
    REQUIRE(result->text == "Hi");
    REQUIRE(result->detectedEncoding == TextEncoding::Utf8Bom);
    REQUIRE(result->hadBom);
}

TEST_CASE("Decode decodes UTF-16 LE with BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {0xFF, 0xFE, 'A', 0x00, 'B', 0x00};
    auto result = Encoding::Decode(data);
    REQUIRE(result.has_value());
    REQUIRE(result->text == "AB");
    REQUIRE(result->detectedEncoding == TextEncoding::Utf16Le);
}

TEST_CASE("Decode decodes UTF-16 BE with BOM", "[encoding]") {
    std::vector<std::uint8_t> data = {0xFE, 0xFF, 0x00, 'A', 0x00, 'B'};
    auto result = Encoding::Decode(data);
    REQUIRE(result.has_value());
    REQUIRE(result->text == "AB");
    REQUIRE(result->detectedEncoding == TextEncoding::Utf16Be);
}

TEST_CASE("Decode handles empty data", "[encoding]") {
    std::vector<std::uint8_t> data;
    auto result = Encoding::Decode(data);
    REQUIRE(result.has_value());
    REQUIRE(result->text == "");
}

TEST_CASE("Encode encodes UTF-8", "[encoding]") {
    auto result = Encoding::Encode("Hello", TextEncoding::Utf8);
    REQUIRE(result.has_value());
    std::string text(result->bytes.begin(), result->bytes.end());
    REQUIRE(text == "Hello");
}

TEST_CASE("Encode encodes UTF-8 with BOM", "[encoding]") {
    auto result = Encoding::Encode("Hi", TextEncoding::Utf8Bom);
    REQUIRE(result.has_value());
    REQUIRE(result->bytes[0] == 0xEF);
    REQUIRE(result->bytes[1] == 0xBB);
    REQUIRE(result->bytes[2] == 0xBF);
    std::string text(result->bytes.begin() + 3, result->bytes.end());
    REQUIRE(text == "Hi");
}

TEST_CASE("Encode encodes UTF-16 LE with BOM", "[encoding]") {
    auto result = Encoding::Encode("AB", TextEncoding::Utf16Le);
    REQUIRE(result.has_value());
    REQUIRE(result->bytes[0] == 0xFF);
    REQUIRE(result->bytes[1] == 0xFE);
    REQUIRE(result->bytes[2] == 'A');
    REQUIRE(result->bytes[3] == 0x00);
    REQUIRE(result->bytes[4] == 'B');
    REQUIRE(result->bytes[5] == 0x00);
}

TEST_CASE("Encode encodes UTF-16 BE with BOM", "[encoding]") {
    auto result = Encoding::Encode("AB", TextEncoding::Utf16Be);
    REQUIRE(result.has_value());
    REQUIRE(result->bytes[0] == 0xFE);
    REQUIRE(result->bytes[1] == 0xFF);
    REQUIRE(result->bytes[2] == 0x00);
    REQUIRE(result->bytes[3] == 'A');
    REQUIRE(result->bytes[4] == 0x00);
    REQUIRE(result->bytes[5] == 'B');
}

TEST_CASE("EncodingName returns correct names", "[encoding]") {
    REQUIRE(Encoding::EncodingName(TextEncoding::Utf8) == "UTF-8");
    REQUIRE(Encoding::EncodingName(TextEncoding::Utf8Bom) == "UTF-8 (BOM)");
    REQUIRE(Encoding::EncodingName(TextEncoding::Utf16Le) == "UTF-16 LE");
    REQUIRE(Encoding::EncodingName(TextEncoding::Utf16Be) == "UTF-16 BE");
    REQUIRE(Encoding::EncodingName(TextEncoding::Ascii) == "ASCII");
    REQUIRE(Encoding::EncodingName(TextEncoding::Unknown) == "Unknown");
}

TEST_CASE("HasBom returns correct values", "[encoding]") {
    REQUIRE(Encoding::HasBom(TextEncoding::Utf8Bom));
    REQUIRE(Encoding::HasBom(TextEncoding::Utf16Le));
    REQUIRE(Encoding::HasBom(TextEncoding::Utf16Be));
    REQUIRE(!Encoding::HasBom(TextEncoding::Utf8));
    REQUIRE(!Encoding::HasBom(TextEncoding::Ascii));
    REQUIRE(!Encoding::HasBom(TextEncoding::Unknown));
}

TEST_CASE("Decode rejects invalid UTF-8", "[encoding]") {
    std::vector<std::uint8_t> data = {0xFF, 0xFF, 0xFF, 0xFF};
    auto result = Encoding::Decode(data);
    REQUIRE(!result.has_value());
}

TEST_CASE("Decode rejects overlong UTF-8", "[encoding]") {
    std::vector<std::uint8_t> overlongSlash = {0xC0, 0xAF};
    auto result = Encoding::Decode(overlongSlash);
    REQUIRE(!result.has_value());
}

TEST_CASE("Decode rejects UTF-8 surrogate encodings", "[encoding]") {
    std::vector<std::uint8_t> encodedSurrogate = {0xED, 0xA0, 0x80};
    auto result = Encoding::Decode(encodedSurrogate);
    REQUIRE(!result.has_value());
}

TEST_CASE("Decode rejects out-of-range 4-byte UTF-8 sequence", "[encoding]") {
    std::vector<std::uint8_t> outOfRange = {0xF4, 0x90, 0x80, 0x80};
    auto result = Encoding::Decode(outOfRange);
    REQUIRE(!result.has_value());
}

TEST_CASE("Decode rejects invalid UTF-8 leading byte C1", "[encoding]") {
    std::vector<std::uint8_t> invalidLeading = {0xC1, 0xBF};
    auto result = Encoding::Decode(invalidLeading);
    REQUIRE(!result.has_value());
}

TEST_CASE("UTF-8 round-trip encode/decode", "[encoding]") {
    std::string original = "Hello, World!";
    auto encoded = Encoding::Encode(original, TextEncoding::Utf8);
    REQUIRE(encoded.has_value());

    auto decoded = Encoding::Decode(encoded->bytes);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->text == original);
}
