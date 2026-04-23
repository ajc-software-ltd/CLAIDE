// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        OfficeLayoutBridgeTests.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include <minidocx/model.hpp>

#include "core/office/OfficeDocumentService.hpp"
#include "core/office/OfficeLayoutBridge.hpp"

namespace {

std::filesystem::path GetTempDir() {
    return std::filesystem::temp_directory_path() / "claide_office_layout_bridge_tests";
}

void EnsureTempDir() {
    std::filesystem::create_directories(GetTempDir());
}

void CleanupTempDir() {
    std::error_code ec;
    std::filesystem::remove_all(GetTempDir(), ec);
}

std::filesystem::path CreateDocxFixture(const std::string& baseName, const std::vector<std::string>& paragraphs) {
    const auto path = GetTempDir() / (baseName + ".docx");

    md::Document document;
    auto section = document.addSection();
    for (const auto& paragraphText : paragraphs) {
        auto paragraph = section->addParagraph();
        paragraph->addRichText(paragraphText);
    }

    document.saveAs(path.string());
    return path;
}

} // namespace

TEST_CASE("OfficeLayoutBridge builds snapshot from service open result", "[office][layout]") {
    EnsureTempDir();
    const auto inputPath = CreateDocxFixture("service_to_layout", {"First paragraph", "Second paragraph"});

    Core::Office::OfficeDocumentService service;
    auto openResult = service.OpenDocx(inputPath);
    REQUIRE(openResult.has_value());

    Core::Office::OfficeLayoutBridge bridge;
    auto snapshotResult = bridge.BuildSnapshot(*openResult);

    REQUIRE(snapshotResult.has_value());
    REQUIRE(snapshotResult->paragraphCount == snapshotResult->paragraphs.size());
    REQUIRE(snapshotResult->paragraphCount >= 2);
    REQUIRE(snapshotResult->totalTextLength == openResult->visibleText.size());

    REQUIRE(snapshotResult->paragraphs[0].paragraphIndex == 0);
    REQUIRE(snapshotResult->paragraphs[0].length > 0);
    REQUIRE(snapshotResult->paragraphs[0].spans.size() == 1);
    REQUIRE(snapshotResult->paragraphs[0].spans[0].text == snapshotResult->paragraphs[0].text);

    CleanupTempDir();
}

TEST_CASE("OfficeLayoutBridge snapshot generation is deterministic", "[office][layout]") {
    EnsureTempDir();
    const auto inputPath = CreateDocxFixture("deterministic_layout", {"Alpha", "Beta", "Gamma"});

    Core::Office::OfficeDocumentService service;
    auto openResult = service.OpenDocx(inputPath);
    REQUIRE(openResult.has_value());

    Core::Office::OfficeLayoutBridge bridge;
    auto firstSnapshot = bridge.BuildSnapshot(*openResult);
    auto secondSnapshot = bridge.BuildSnapshot(*openResult);

    REQUIRE(firstSnapshot.has_value());
    REQUIRE(secondSnapshot.has_value());
    REQUIRE(firstSnapshot->paragraphCount == secondSnapshot->paragraphCount);
    REQUIRE(firstSnapshot->totalTextLength == secondSnapshot->totalTextLength);
    REQUIRE(firstSnapshot->paragraphs.size() == secondSnapshot->paragraphs.size());

    for (size_t index = 0; index < firstSnapshot->paragraphs.size(); ++index) {
        REQUIRE(firstSnapshot->paragraphs[index].paragraphIndex == secondSnapshot->paragraphs[index].paragraphIndex);
        REQUIRE(firstSnapshot->paragraphs[index].startOffset == secondSnapshot->paragraphs[index].startOffset);
        REQUIRE(firstSnapshot->paragraphs[index].length == secondSnapshot->paragraphs[index].length);
        REQUIRE(firstSnapshot->paragraphs[index].text == secondSnapshot->paragraphs[index].text);
    }

    CleanupTempDir();
}

TEST_CASE("OfficeLayoutBridge returns failure for empty visible text", "[office][layout]") {
    Core::Office::OfficeLayoutBridge bridge;
    Core::Office::OpenOfficeDocumentResult openResult;
    openResult.visibleText = "";

    auto snapshotResult = bridge.BuildSnapshot(openResult);

    REQUIRE(!snapshotResult.has_value());
    REQUIRE(snapshotResult.error().find("visible text is empty") != std::string::npos);
}
