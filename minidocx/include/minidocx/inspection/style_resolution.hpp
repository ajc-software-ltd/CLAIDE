#pragma once

#include "config.hpp"
#include "inspection/semantic.hpp"
#include "word/main/paragraph.hpp"
#include "word/main/richtext.hpp"

#include <string>
#include <vector>

namespace MINIDOCX_NAMESPACE::inspection
{
  enum class ResolveIssue
  {
    MissingParagraphStyle,
    MissingCharacterStyle,
    ParagraphStyleCycle,
    CharacterStyleCycle,
    MissingNumberingDefinition,
    MissingAbstractNumberingDefinition,
  };

  struct ResolvedListFormatting
  {
    bool hasNumbering = false;
    NumberingId numberingId = 0;
    NumberingLevel level = NumberingLevel::Level1;
    size_t start = 1;
    NumberStyle style = NumberStyle::Decimal;
    std::string format;
    Alignment align = Alignment::Left;
    ParagraphProperties paragraph;
    RichTextProperties run;
  };

  struct ResolvedParagraphFormatting
  {
    ParagraphProperties paragraph;
    RichTextProperties runDefaults;
    ResolvedListFormatting list;
    std::string paragraphStyleId;
    std::vector<std::string> paragraphStyleChain;
    bool headingLike = false;
    std::vector<ResolveIssue> issues;
  };

  struct ResolvedRunFormatting
  {
    RichTextProperties run;
    std::string characterStyleId;
    std::vector<std::string> characterStyleChain;
    std::vector<ResolveIssue> issues;
  };

  MINIDOCX_API ResolvedParagraphFormatting resolveParagraphFormatting(
      const Document& document, const Paragraph& paragraph);

  MINIDOCX_API ResolvedRunFormatting resolveRunFormatting(
      const Document& document, const Paragraph& paragraph, const RichText& run);

  MINIDOCX_API ResolvedParagraphFormatting resolveParagraphFormatting(
      const Document& document, const NodePath& paragraphPath);

  MINIDOCX_API ResolvedRunFormatting resolveRunFormatting(
      const Document& document, const NodePath& runPath);
}
