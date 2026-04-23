// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        OfficeDocumentServiceTests.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include <minidocx/model.hpp>

#include "core/office/OfficeDocumentService.hpp"

namespace {

std::filesystem::path GetTempDir() {
    return std::filesystem::temp_directory_path() / "claide_office_service_tests";
}

void EnsureTempDir() {
    std::filesystem::create_directories(GetTempDir());
}

void CleanupTempDir() {
    std::error_code ec;
    std::filesystem::remove_all(GetTempDir(), ec);
}

std::filesystem::path CreateDocxFixture(const std::string& baseName, const std::string& text) {
    const auto path = GetTempDir() / (baseName + ".docx");

    md::Document document;
    auto section = document.addSection();
    auto paragraph = section->addParagraph();
    paragraph->addRichText(text);
    document.saveAs(path.string());

    return path;
}

} // namespace

TEST_CASE("OfficeDocumentService opens DOCX and extracts visible text", "[office][docx]") {
    EnsureTempDir();
    const auto inputPath = CreateDocxFixture("open_extract", "Hello DOCX");

    Core::Office::OfficeDocumentService service;
    auto openResult = service.OpenDocx(inputPath);

    REQUIRE(openResult.has_value());
    REQUIRE(openResult->path == inputPath);
    REQUIRE(openResult->visibleText.find("Hello DOCX") != std::string::npos);
    REQUIRE(openResult->summary.paragraphCount >= 1);
    REQUIRE(openResult->summary.runCount >= 1);

    CleanupTempDir();
}

TEST_CASE("OfficeDocumentService saves DOCX after open", "[office][docx]") {
    EnsureTempDir();
    const auto inputPath = CreateDocxFixture("save_input", "Roundtrip");
    const auto outputPath = GetTempDir() / "save_output.docx";

    Core::Office::OfficeDocumentService service;
    auto openResult = service.OpenDocx(inputPath);
    REQUIRE(openResult.has_value());

    auto saveResult = service.SaveDocx(outputPath);
    REQUIRE(saveResult.has_value());
    REQUIRE(saveResult->path == outputPath);
    REQUIRE(std::filesystem::exists(outputPath));

    CleanupTempDir();
}

TEST_CASE("OfficeDocumentService returns failure for missing file", "[office][docx]") {
    CleanupTempDir();

    Core::Office::OfficeDocumentService service;
    auto openResult = service.OpenDocx(GetTempDir() / "missing.docx");

    REQUIRE(!openResult.has_value());
    REQUIRE(!openResult.error().empty());
}

TEST_CASE("OfficeDocumentService save fails before open", "[office][docx]") {
    CleanupTempDir();

    Core::Office::OfficeDocumentService service;
    auto saveResult = service.SaveDocx(GetTempDir() / "no_loaded_document.docx");

    REQUIRE(!saveResult.has_value());
    REQUIRE(saveResult.error().find("no document loaded") != std::string::npos);
}
