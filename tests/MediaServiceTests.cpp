// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        MediaServiceTests.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "core/MediaService.hpp"
#include "platform/PlatformPaths.hpp"

using namespace Core;

TEST_CASE("DetectMediaType detects image files", "[mediaservice]") {
    REQUIRE(MediaService::DetectMediaType("test.png") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.jpg") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.jpeg") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.bmp") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.gif") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.tiff") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.webp") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.psd") == MediaType::Image);
    REQUIRE(MediaService::DetectMediaType("test.psb") == MediaType::Image);
}

TEST_CASE("DetectMediaType detects video files", "[mediaservice]") {
    REQUIRE(MediaService::DetectMediaType("test.mp4") == MediaType::Video);
    REQUIRE(MediaService::DetectMediaType("test.webm") == MediaType::Video);
    REQUIRE(MediaService::DetectMediaType("test.mkv") == MediaType::Video);
    REQUIRE(MediaService::DetectMediaType("test.avi") == MediaType::Video);
    REQUIRE(MediaService::DetectMediaType("test.mov") == MediaType::Video);
}

TEST_CASE("DetectMediaType detects audio files", "[mediaservice]") {
    REQUIRE(MediaService::DetectMediaType("test.mp3") == MediaType::Audio);
    REQUIRE(MediaService::DetectMediaType("test.wav") == MediaType::Audio);
    REQUIRE(MediaService::DetectMediaType("test.ogg") == MediaType::Audio);
    REQUIRE(MediaService::DetectMediaType("test.flac") == MediaType::Audio);
}

TEST_CASE("DetectMediaType detects model files", "[mediaservice]") {
    REQUIRE(MediaService::DetectMediaType("test.fbx") == MediaType::Model);
    REQUIRE(MediaService::DetectMediaType("test.obj") == MediaType::Model);
    REQUIRE(MediaService::DetectMediaType("test.gltf") == MediaType::Model);
    REQUIRE(MediaService::DetectMediaType("test.glb") == MediaType::Model);
}

TEST_CASE("DetectMediaType detects text files", "[mediaservice]") {
    REQUIRE(MediaService::DetectMediaType("test.txt") == MediaType::Text);
    REQUIRE(MediaService::DetectMediaType("test.cpp") == MediaType::Text);
    REQUIRE(MediaService::DetectMediaType("test.md") == MediaType::Text);
    REQUIRE(MediaService::DetectMediaType("test.json") == MediaType::Text);
}

TEST_CASE("DetectMediaType returns Unknown for unsupported extensions", "[mediaservice]") {
    REQUIRE(MediaService::DetectMediaType("test.xyz") == MediaType::Unknown);
    REQUIRE(MediaService::DetectMediaType("test.dat") == MediaType::Unknown);
}

TEST_CASE("MediaTypeToString returns correct names", "[mediaservice]") {
    REQUIRE(MediaService::MediaTypeToString(MediaType::Image) == "Image");
    REQUIRE(MediaService::MediaTypeToString(MediaType::Video) == "Video");
    REQUIRE(MediaService::MediaTypeToString(MediaType::Audio) == "Audio");
    REQUIRE(MediaService::MediaTypeToString(MediaType::Model) == "Model");
    REQUIRE(MediaService::MediaTypeToString(MediaType::Text) == "Text");
    REQUIRE(MediaService::MediaTypeToString(MediaType::Unknown) == "Unknown");
}

TEST_CASE("IsImageFile returns correct values", "[mediaservice]") {
    REQUIRE(MediaService::IsImageFile("photo.PNG"));
    REQUIRE(MediaService::IsImageFile("photo.jpg"));
    REQUIRE(!MediaService::IsImageFile("video.mp4"));
}

TEST_CASE("GetImageMetadata reads a valid PNG file", "[mediaservice]") {
    auto path = Platform::GetProjectRoot() / "assets" / "canvas.png";

    if (!std::filesystem::exists(path)) {
        SKIP("canvas.png not found");
    }

    auto meta = MediaService::GetImageMetadata(path);
    REQUIRE(meta.has_value());
    REQUIRE(meta->width > 0);
    REQUIRE(meta->height > 0);
    REQUIRE(meta->format == "PNG");
    REQUIRE(meta->fileSize > 0);
}

TEST_CASE("GetImageMetadata returns error for non-existent file", "[mediaservice]") {
    auto path = std::filesystem::path("/nonexistent/test.png");
    auto meta = MediaService::GetImageMetadata(path);
    REQUIRE(!meta.has_value());
}
