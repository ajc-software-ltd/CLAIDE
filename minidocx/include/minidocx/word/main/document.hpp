// ============================================================================
// MINIDOCX
// ============================================================================
// File:        document.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "config.hpp"
#include "packaging/package.hpp"
#include "word/styles.hpp"
#include "word/numbering.hpp"

#include <memory>
#include <string>
#include <iosfwd>
#include <list>
#include <map>


namespace MINIDOCX_NAMESPACE
{
  class Section;
  class Paragraph;
  class Table;
  class Picture;

  using SectionPointer = std::shared_ptr<Section>;

  class MINIDOCX_API Document : public Package
  {
  public:
    Document();
    void saveAs(const std::string& filename);
    void saveToStream(std::ostream& stream);
    Buffer saveToBuffer();

    void load(const std::string& filename);
    void loadFromStream(std::istream& stream);
    void loadFromBuffer(const Buffer& buffer);
    inline void save() { saveAs(filename_); }

    inline void reset() { clear(); init(); }

  private:
    void init();
    void flush();
    void clear();
    void load();

  private:
    PartName mainPart_{ MAIN_PART };

    void initContentTypes();
    void initRelationships();

    void writeOfficeDocument();
    void readOfficeDocument();

  private:
    std::list<SectionPointer> sections_;

  public:
    inline std::list<SectionPointer> sections() const { return sections_; }
    inline const std::map<std::string, ParagraphStyle>& paragraphStyles() const { return paragraphStyles_; }
    inline const std::map<std::string, CharacterStyle>& characterStyles() const { return characterStyles_; }
    inline const std::map<NumberingId, AbstractNumberingDefinition>& abstractNumberingDefinitions() const { return abstractNumDefinitions_; }
    inline const std::map<NumberingId, NumberingDefinition>& numberingDefinitions() const { return numDefinitions_; }
    SectionPointer addSection();
    void deleteSection(const SectionPointer& section);
    void clearSections();

  private:
    size_t imgCount_ = 0;

  public:
    RelationshipId addImage(Buffer buf, const FileType type);
    RelationshipId addImage(const FileName& filename);

  private:
    PartName stylePart_{ STYLE_PART };

    std::map<std::string, ParagraphStyle> paragraphStyles_;
    std::map<std::string, CharacterStyle> characterStyles_;

    void writeStyles();
    void readStyles();

  public:
    void addParagraphStyle(const ParagraphStyle& style);
    void addCharacterStyle(const CharacterStyle& style);

  private:
    PartName numPart_{ NUM_PART };

    NumberingId nextAbstractNumId_ = 0;
    NumberingId nextNumId_ = 1;
    std::map<NumberingId, AbstractNumberingDefinition> abstractNumDefinitions_;
    std::map<NumberingId, NumberingDefinition> numDefinitions_;

    void writeNumDefinitions();
    void readNumDefinitions();

  public:
    // Adds abstract numbering definition.
    NumberingId addAbstractNumDefinition(AbstractNumberingDefinition def)
    {
      const NumberingId numId = nextAbstractNumId_++;
      abstractNumDefinitions_[numId] = std::move(def);
      return numId;
    }

    // Adds numbering definition instance.
    NumberingId addNumDefinition(NumberingDefinition def)
    {
      const NumberingId numId = nextNumId_++;
      numDefinitions_[numId] = std::move(def);
      return numId;
    }

    // Adds numbering definition instance for numbered list.
    inline NumberingId addNumberedListDefinition()
    {
      return addNumDefinition(
        addAbstractNumDefinition(AbstractNumberingDefinition::makeNumberedList()));
    }

    // Adds numbering definition instance for bulleted list.
    inline NumberingId addBulletedListDefinition()
    {
      return addNumDefinition(
        addAbstractNumDefinition(AbstractNumberingDefinition::makeBulletedList()));
    }
  };
}
