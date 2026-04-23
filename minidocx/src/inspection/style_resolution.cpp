#include "inspection/style_resolution.hpp"

#include "utils/string.hpp"

#include <set>
#include <stdexcept>

namespace MINIDOCX_NAMESPACE::inspection
{
  namespace
  {
    template <typename T>
    void applyOptional(std::optional<T>& target, const std::optional<T>& source)
    {
      if (source.has_value())
        target = source;
    }

    void applyParagraphDelta(ParagraphProperties& target, const ParagraphProperties& source)
    {
      if (!source.style_.empty())
        target.style_ = source.style_;
      applyOptional(target.align_, source.align_);
      if (source.outlineLevel_ != ParagraphProperties::OutlineLevel::BodyText)
        target.outlineLevel_ = source.outlineLevel_;
      applyOptional(target.indent_, source.indent_);
      applyOptional(target.spacing_, source.spacing_);
      applyOptional(target.borders_, source.borders_);
      if (source.keepNext_)
        target.keepNext_ = true;
      if (source.keepLines_)
        target.keepLines_ = true;
      if (source.pageBreakBefore_)
        target.pageBreakBefore_ = true;
      if (source.pageBreakAfter_)
        target.pageBreakAfter_ = true;
    }

    void applyRichTextDelta(RichTextProperties& target, const RichTextProperties& source)
    {
      if (!source.style_.empty())
        target.style_ = source.style_;
      applyOptional(target.font_, source.font_);
      if (source.fontStyle_.bold_)
        target.fontStyle_.bold_ = true;
      if (source.fontStyle_.italic_)
        target.fontStyle_.italic_ = true;
      if (source.fontSize_ != 0)
        target.fontSize_ = source.fontSize_;
      if (!source.color_.empty())
        target.color_ = source.color_;
      if (source.underline_.style_ != RichTextProperties::UnderlineStyle::None)
        target.underline_ = source.underline_;
      if (source.effects_.strike_ != RichTextProperties::StrikeStyle::None)
        target.effects_.strike_ = source.effects_.strike_;
      if (source.effects_.vertAlign_ != RichTextProperties::VertAlign::None)
        target.effects_.vertAlign_ = source.effects_.vertAlign_;
      if (source.highlight_ != RichTextProperties::Highlight::None)
        target.highlight_ = source.highlight_;
      if (source.scale_ != 100)
        target.scale_ = source.scale_;
      if (source.spacing_.type_ != RichTextProperties::SpacingType::Normal || source.spacing_.by_ != 20)
        target.spacing_ = source.spacing_;
      if (source.position_.type_ != RichTextProperties::PositionType::Normal || source.position_.by_ != 2)
        target.position_ = source.position_;
      applyOptional(target.border_, source.border_);
      if (source.whitespace_)
        target.whitespace_ = true;
    }

    const ParagraphStyle* findParagraphStyleById(const Document& document, const std::string& styleId)
    {
      const auto& styles = document.paragraphStyles();
      auto it = styles.find(styleId);
      if (it != styles.end())
        return &it->second;
      return nullptr;
    }

    const CharacterStyle* findCharacterStyleById(const Document& document, const std::string& styleId)
    {
      const auto& styles = document.characterStyles();
      auto it = styles.find(styleId);
      if (it != styles.end())
        return &it->second;
      return nullptr;
    }

    void resolveParagraphStyleChain(
        const Document& document,
        const std::string& startStyleId,
        ParagraphProperties& paragraph,
        RichTextProperties& runDefaults,
        std::vector<std::string>& chain,
        std::vector<ResolveIssue>& issues)
    {
      if (startStyleId.empty())
        return;

      std::set<std::string> visited;
      std::vector<const ParagraphStyle*> stack;
      std::string current = removeSpaces(startStyleId);

      while (!current.empty()) {
        const ParagraphStyle* style = findParagraphStyleById(document, current);
        if (style == nullptr) {
          issues.push_back(ResolveIssue::MissingParagraphStyle);
          break;
        }

        if (!visited.insert(current).second) {
          issues.push_back(ResolveIssue::ParagraphStyleCycle);
          break;
        }

        chain.push_back(current);
        stack.push_back(style);
        current = removeSpaces(style->basedOn_);
      }

      for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        applyParagraphDelta(paragraph, **it);
        applyRichTextDelta(runDefaults, **it);
      }
    }

    void resolveCharacterStyleChain(
        const Document& document,
        const std::string& startStyleId,
        RichTextProperties& run,
        std::vector<std::string>& chain,
        std::vector<ResolveIssue>& issues)
    {
      if (startStyleId.empty())
        return;

      std::set<std::string> visited;
      std::vector<const CharacterStyle*> stack;
      std::string current = removeSpaces(startStyleId);

      while (!current.empty()) {
        const CharacterStyle* style = findCharacterStyleById(document, current);
        if (style == nullptr) {
          issues.push_back(ResolveIssue::MissingCharacterStyle);
          break;
        }

        if (!visited.insert(current).second) {
          issues.push_back(ResolveIssue::CharacterStyleCycle);
          break;
        }

        chain.push_back(current);
        stack.push_back(style);
        current = removeSpaces(style->basedOn_);
      }

      for (auto it = stack.rbegin(); it != stack.rend(); ++it)
        applyRichTextDelta(run, **it);
    }

    const LevelDefinition* resolveLevelDefinition(const Document& document, const Paragraph& paragraph, std::vector<ResolveIssue>& issues)
    {
      if (paragraph.numId_ == 0)
        return nullptr;

      const auto numIt = document.numberingDefinitions().find(paragraph.numId_);
      if (numIt == document.numberingDefinitions().end()) {
        issues.push_back(ResolveIssue::MissingNumberingDefinition);
        return nullptr;
      }

      const NumberingDefinition& def = numIt->second;
      const auto levelOverrideIt = def.levelOverrides_.find(paragraph.level_);
      if (levelOverrideIt != def.levelOverrides_.end())
        return &levelOverrideIt->second;

      const auto absIt = document.abstractNumberingDefinitions().find(def.id_);
      if (absIt == document.abstractNumberingDefinitions().end()) {
        issues.push_back(ResolveIssue::MissingAbstractNumberingDefinition);
        return nullptr;
      }

      return &absIt->second.levels_[static_cast<size_t>(paragraph.level_)];
    }

    const Paragraph& paragraphAt(const Document& document, const NodePath& paragraphPath)
    {
      const auto sections = document.sections();
      if (paragraphPath.sectionIndex >= sections.size())
        throw std::out_of_range("section index out of range");
      auto secIt = sections.begin();
      std::advance(secIt, static_cast<long>(paragraphPath.sectionIndex));

      const auto blocks = (*secIt)->blocks();
      if (paragraphPath.blockIndex >= blocks.size())
        throw std::out_of_range("block index out of range");
      auto blockIt = blocks.begin();
      std::advance(blockIt, static_cast<long>(paragraphPath.blockIndex));
      if ((*blockIt)->type() != BlockType::Paragraph)
        throw std::invalid_argument("node path does not reference a paragraph");

      return *std::dynamic_pointer_cast<Paragraph>(*blockIt);
    }

    const RichText& richTextAt(const Document& document, const NodePath& runPath, const Paragraph*& paragraphOut)
    {
      const Paragraph& paragraph = paragraphAt(document, runPath);
      paragraphOut = &paragraph;
      const auto runs = paragraph.runs();
      if (!runPath.hasRunIndex || runPath.runIndex >= runs.size())
        throw std::out_of_range("run index out of range");
      auto runIt = runs.begin();
      std::advance(runIt, static_cast<long>(runPath.runIndex));
      if ((*runIt)->type() != RunType::RichText)
        throw std::invalid_argument("node path does not reference a rich text run");
      return *std::dynamic_pointer_cast<RichText>(*runIt);
    }
  }

  ResolvedParagraphFormatting resolveParagraphFormatting(const Document& document, const Paragraph& paragraph)
  {
    ResolvedParagraphFormatting resolved;

    if (!paragraph.prop_.style_.empty()) {
      resolved.paragraphStyleId = removeSpaces(paragraph.prop_.style_);
      resolveParagraphStyleChain(
          document, resolved.paragraphStyleId, resolved.paragraph, resolved.runDefaults, resolved.paragraphStyleChain, resolved.issues);
    }

    if (const LevelDefinition* level = resolveLevelDefinition(document, paragraph, resolved.issues); level != nullptr) {
      resolved.list.hasNumbering = true;
      resolved.list.numberingId = paragraph.numId_;
      resolved.list.level = paragraph.level_;
      resolved.list.start = level->numStart_;
      resolved.list.style = level->numStyle_;
      resolved.list.format = level->numFmt_;
      resolved.list.align = level->numAlign_;
      resolved.list.paragraph = *level;
      resolved.list.run = *level;
      applyParagraphDelta(resolved.paragraph, resolved.list.paragraph);
      applyRichTextDelta(resolved.runDefaults, resolved.list.run);
    }

    applyParagraphDelta(resolved.paragraph, paragraph.prop_);
    resolved.headingLike = resolved.paragraph.outlineLevel_ != ParagraphProperties::OutlineLevel::BodyText;
    return resolved;
  }

  ResolvedRunFormatting resolveRunFormatting(const Document& document, const Paragraph& paragraph, const RichText& run)
  {
    ResolvedRunFormatting resolved;

    const ResolvedParagraphFormatting paragraphResolved = resolveParagraphFormatting(document, paragraph);
    resolved.run = paragraphResolved.runDefaults;
    resolved.issues = paragraphResolved.issues;

    if (!run.prop_.style_.empty()) {
      resolved.characterStyleId = removeSpaces(run.prop_.style_);
      resolveCharacterStyleChain(document, resolved.characterStyleId, resolved.run, resolved.characterStyleChain, resolved.issues);
    }

    applyRichTextDelta(resolved.run, run.prop_);
    return resolved;
  }

  ResolvedParagraphFormatting resolveParagraphFormatting(const Document& document, const NodePath& paragraphPath)
  {
    return resolveParagraphFormatting(document, paragraphAt(document, paragraphPath));
  }

  ResolvedRunFormatting resolveRunFormatting(const Document& document, const NodePath& runPath)
  {
    const Paragraph* paragraph = nullptr;
    const RichText& run = richTextAt(document, runPath, paragraph);
    return resolveRunFormatting(document, *paragraph, run);
  }
}
