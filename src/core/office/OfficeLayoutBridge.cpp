// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        OfficeLayoutBridge.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/office/OfficeLayoutBridge.hpp"

#include <sstream>
#include <utility>

namespace Core::Office {

std::expected<OfficeLayoutSnapshot, std::string>
OfficeLayoutBridge::BuildSnapshot(const OpenOfficeDocumentResult& openResult) const {
    if (openResult.visibleText.empty()) {
        return std::unexpected("OfficeLayoutBridge::BuildSnapshot failed: visible text is empty");
    }

    OfficeLayoutSnapshot snapshot;
    size_t runningOffset = 0;

    std::stringstream stream(openResult.visibleText);
    std::string paragraphText;
    while (std::getline(stream, paragraphText, '\n')) {
        if (paragraphText.empty()) {
            runningOffset += 1;
            continue;
        }

        OfficeLayoutParagraph paragraph;
        paragraph.paragraphIndex = snapshot.paragraphs.size();
        paragraph.startOffset = runningOffset;
        paragraph.length = paragraphText.size();
        paragraph.text = paragraphText;
        paragraph.spans.push_back(OfficeLayoutSpan{.startOffset = paragraph.startOffset,
                                                   .length = paragraph.length,
                                                   .text = paragraphText});

        snapshot.paragraphs.push_back(std::move(paragraph));
        runningOffset += paragraphText.size() + 1;
    }

    if (snapshot.paragraphs.empty()) {
        return std::unexpected("OfficeLayoutBridge::BuildSnapshot failed: no non-empty paragraphs found");
    }

    snapshot.paragraphCount = snapshot.paragraphs.size();
    snapshot.totalTextLength = openResult.visibleText.size();
    return snapshot;
}

} // namespace Core::Office
