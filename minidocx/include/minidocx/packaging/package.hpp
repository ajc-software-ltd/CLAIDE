// ============================================================================
// MINIDOCX
// ============================================================================
// File:        package.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "utils/zip.hpp"
#include "utils/file.hpp"
#include "packaging/part.hpp"
#include "packaging/relationship.hpp"

#include <string>
#include <map>
#include <vector>
#include <optional>


namespace pugi { class xml_document; }

namespace MINIDOCX_NAMESPACE
{
  struct PackageProperties
  {
    std::string title_;
    std::string subject_;
    std::string author_;
    std::string company_;
    std::string lastModifiedBy_;
  };
  

  class Package : protected Zip
  {
  public:
    PackageProperties prop_;

    std::string readPartText(const PartName& name)
    {
      return extractFileToString(name);
    }

    Buffer readPartBinary(const PartName& name)
    {
      const std::string raw = extractFileToString(name);
      return Buffer(raw.begin(), raw.end());
    }

  protected:
    void init();
    void flush();
    void clear();
    void load();

    void writePart(const PartName& name, pugi::xml_document& doc);

  private:
    std::map<PartName, Buffer> buffered_;

    void writeBufferedParts();

  protected:
    void addBufferedPart(const PartName& name, Buffer buf)
    {
      buffered_[name] = std::move(buf);
    }

  private:
    using MediaType = std::string;

    std::map<PartExtension, MediaType> defaultContentTypes_;
    std::map<PartName, MediaType> overrideContentTypes_;

    void initContentTypes();
    void writeContentTypes();
    void readContentTypes();

  protected:
    inline void registerDefaultContentType(
      const PartExtension& ext, const MediaType& type)
    {
      defaultContentTypes_[ext] = type;
    }

    inline void registerOverrideContentType(
      const PartName& name, const MediaType& type)
    {
      overrideContentTypes_[name] = type;
    }

    inline void unregisterOverrideContentType(const PartName& name)
    {
      overrideContentTypes_.erase(name);
    }

  private:
    std::map<PartName, Relationships> relationships_;
    Relationships packageRelationships_;

    inline RelationshipId addRelationshipTo(Relationships& rels,
      const PartType type, const PartName& target, const Relationship::TargetMode mode)
    {
      // TODO: Check if the target already exists.
      const RelationshipId id = ++rels.maxId_;
      rels.map_[id] = { id, type, target, mode };
      return id;
    }

    inline void deleteRelationshipFrom(Relationships& rels, const RelationshipId id)
    {
      rels.map_.erase(id);
    }

    void writeRelationships(Relationships& rels, const PartName& name);
    void writeRelationships();

    void readRelationships(const PartName& name, Relationships& rels);
    inline void readPkgRelationships()
    {
      readRelationships(RELS_PART, packageRelationships_);
    }

  protected:
    inline RelationshipId addRelationshipFor(const PartName& src,
      const PartType type, const PartName& target, const Relationship::TargetMode mode)
    {
      return addRelationshipTo(relationships_[src], type, fs::relative(target, src.parent_path()), mode);
    }

    inline RelationshipId addPkgRelationship(
      const PartType type, const PartName& target, const Relationship::TargetMode mode)
    {
      return addRelationshipTo(packageRelationships_, type, target.relative_path(), mode);
    }

    inline void deleteRelationshipFor(const PartName& src, const RelationshipId id)
    {
      deleteRelationshipFrom(relationships_[src], id);
    }

    inline void deletePkgRelationship(const RelationshipId id)
    {
      deleteRelationshipFrom(packageRelationships_, id);
    }

    inline void initRelationshipsFor(const PartName& src)
    {
      relationships_[src] = Relationships();
    }

    inline void readRelationshipsFor(const PartName& src)
    {
      initRelationshipsFor(src);
      readRelationships(toRelationshipsPartName(src), relationships_[src]);
    }


  public:
    std::optional<Relationship> findRelationshipFor(const PartName& src, const RelationshipId id) const
    {
      const auto relsIt = relationships_.find(src);
      if (relsIt == relationships_.end())
        return std::nullopt;

      const auto relIt = relsIt->second.map_.find(id);
      if (relIt == relsIt->second.map_.end())
        return std::nullopt;

      return relIt->second;
    }

    std::optional<Relationship> findPackageRelationship(const RelationshipId id) const
    {
      const auto relIt = packageRelationships_.map_.find(id);
      if (relIt == packageRelationships_.map_.end())
        return std::nullopt;
      return relIt->second;
    }

  private:
    PartName corePart_{ CORE_PART };
    PartName appPart_{ APP_PART };

    void writeCoreProperties();
    void writeExtendedProperties();

    void readCoreProperties();
    void readExtendedProperties();
  };
}
