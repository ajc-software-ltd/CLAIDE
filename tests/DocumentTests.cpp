// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        DocumentTests.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "core/Document.hpp"

using namespace Core;

TEST_CASE("New document is untitled and unmodified", "[document]") {
    Document doc;
    REQUIRE(doc.IsUntitled());
    REQUIRE(!doc.IsModified());
    REQUIRE(doc.GetContent() == "");
    REQUIRE(doc.GetEncoding() == TextEncoding::Utf8);
}

TEST_CASE("SetContent updates content and clears modified", "[document]") {
    Document doc;
    doc.SetContent("Hello");
    REQUIRE(doc.GetContent() == "Hello");
    REQUIRE(!doc.IsModified());
}

TEST_CASE("SetModified tracks dirty state", "[document]") {
    Document doc;
    doc.SetContent("Initial");
    REQUIRE(!doc.IsModified());

    doc.SetModified(true);
    REQUIRE(doc.IsModified());

    doc.SetModified(false);
    REQUIRE(!doc.IsModified());
}

TEST_CASE("SetFilePath clears untitled state", "[document]") {
    Document doc;
    REQUIRE(doc.IsUntitled());

    doc.SetFilePath(std::filesystem::path("/tmp/test.txt"));
    REQUIRE(!doc.IsUntitled());
    REQUIRE(doc.GetFilePath().has_value());
    REQUIRE(*doc.GetFilePath() == std::filesystem::path("/tmp/test.txt"));
}

TEST_CASE("GetDisplayName returns filename for saved doc", "[document]") {
    Document doc;
    REQUIRE(doc.GetDisplayName() == "Untitled");

    doc.SetFilePath(std::filesystem::path("/home/user/notes.txt"));
    REQUIRE(doc.GetDisplayName() == "notes.txt");
}

TEST_CASE("SetEncoding tracks encoding", "[document]") {
    Document doc;
    REQUIRE(doc.GetEncoding() == TextEncoding::Utf8);

    doc.SetEncoding(TextEncoding::Utf8Bom);
    REQUIRE(doc.GetEncoding() == TextEncoding::Utf8Bom);

    doc.SetEncoding(TextEncoding::Utf16Le);
    REQUIRE(doc.GetEncoding() == TextEncoding::Utf16Le);
}

TEST_CASE("Clear resets document to initial state", "[document]") {
    Document doc;
    doc.SetContent("Some content");
    doc.SetFilePath(std::filesystem::path("/tmp/test.txt"));
    doc.SetModified(true);
    doc.SetEncoding(TextEncoding::Utf16Be);

    doc.Clear();

    REQUIRE(doc.GetContent() == "");
    REQUIRE(doc.IsUntitled());
    REQUIRE(!doc.IsModified());
    REQUIRE(doc.GetEncoding() == TextEncoding::Utf8);
}

TEST_CASE("Document lifecycle: create, modify, save, clear", "[document]") {
    Document doc;

    REQUIRE(doc.IsUntitled());
    REQUIRE(doc.GetDisplayName() == "Untitled");

    doc.SetContent("Draft text");
    doc.SetModified(true);
    REQUIRE(doc.IsModified());

    doc.SetFilePath(std::filesystem::path("/tmp/final.txt"));
    REQUIRE(!doc.IsUntitled());
    REQUIRE(doc.GetDisplayName() == "final.txt");

    doc.SetModified(false);
    REQUIRE(!doc.IsModified());

    doc.Clear();
    REQUIRE(doc.IsUntitled());
    REQUIRE(doc.GetContent() == "");
}
