#include "miniexcelx/Workbook.hpp"

#include "minidocx/config.hpp"
#include "minidocx/utils/zip.hpp"
#include "pugixml.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace miniexcelx {
namespace {

[[nodiscard]] std::string toLower(std::string_view text) {
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

[[nodiscard]] bool hasExtension(std::filesystem::path path, std::string_view extension) {
    return toLower(path.extension().string()) == extension;
}

[[nodiscard]] std::string normalizeArchivePath(std::string raw) {
    std::replace(raw.begin(), raw.end(), '\\', '/');
    if (raw.empty()) {
        return raw;
    }

    if (raw.front() != '/') {
        raw.insert(raw.begin(), '/');
    }

    std::filesystem::path normalized(raw);
    return normalized.lexically_normal().generic_string();
}

[[nodiscard]] std::string resolveRelativeTarget(std::string_view sourcePart, std::string_view target) {
    if (!target.empty() && target.front() == '/') {
        return normalizeArchivePath(std::string(target));
    }

    std::filesystem::path base(sourcePart);
    base = base.parent_path();
    std::filesystem::path combined = base / std::string(target);
    return normalizeArchivePath(combined.generic_string());
}

[[nodiscard]] std::string stripPrefix(std::string_view value, std::string_view prefix) {
    if (value.substr(0, prefix.size()) == prefix) {
        return std::string(value.substr(prefix.size()));
    }

    return std::string(value);
}

[[nodiscard]] std::string_view localName(std::string_view qualifiedName) {
    const std::size_t separator = qualifiedName.find(':');
    if (separator == std::string_view::npos) {
        return qualifiedName;
    }

    return qualifiedName.substr(separator + 1);
}

[[nodiscard]] bool extractFileToString(MINIDOCX_NAMESPACE::Zip& archive, std::string_view canonicalPath, std::string& outContent) {
    const std::string withSlash(canonicalPath);
    const std::string withoutSlash = stripPrefix(withSlash, "/");

    if (archive.hasEntry(withSlash)) {
        outContent = archive.extractFileToString(withSlash);
        return true;
    }

    if (archive.hasEntry(withoutSlash)) {
        outContent = archive.extractFileToString(withoutSlash);
        return true;
    }

    return false;
}

[[nodiscard]] std::optional<std::string> findWorkbookPartFromContentTypes(const pugi::xml_document& contentTypesDoc) {
    const pugi::xml_node root = contentTypesDoc.document_element();
    for (const pugi::xml_node child : root.children()) {
        if (localName(child.name()) != "Override") {
            continue;
        }

        const std::string_view contentType = child.attribute("ContentType").as_string();
        if (contentType != "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml") {
            continue;
        }

        const std::string partName = child.attribute("PartName").as_string();
        if (!partName.empty()) {
            return normalizeArchivePath(partName);
        }
    }

    return std::nullopt;
}

[[nodiscard]] std::optional<std::string> findOfficeDocumentFromRootRels(const pugi::xml_document& rootRelsDoc) {
    const pugi::xml_node root = rootRelsDoc.document_element();
    for (const pugi::xml_node child : root.children()) {
        if (localName(child.name()) != "Relationship") {
            continue;
        }

        const std::string_view relationshipType = child.attribute("Type").as_string();
        if (relationshipType != "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument") {
            continue;
        }

        const std::string target = child.attribute("Target").as_string();
        if (!target.empty()) {
            return resolveRelativeTarget("/", target);
        }
    }

    return std::nullopt;
}

[[nodiscard]] std::string workbookRelsPath(std::string_view workbookPartPath) {
    std::filesystem::path workbookPath(workbookPartPath);
    const std::string fileName = workbookPath.filename().string();
    std::filesystem::path relsPath = workbookPath.parent_path() / "_rels" / (fileName + ".rels");
    return normalizeArchivePath(relsPath.generic_string());
}

[[nodiscard]] std::unordered_map<std::string, std::string> parseWorksheetTargets(const pugi::xml_document& workbookRelsDoc,
                                                                                  std::string_view workbookPartPath) {
    std::unordered_map<std::string, std::string> targetsById;

    const pugi::xml_node root = workbookRelsDoc.document_element();
    for (const pugi::xml_node child : root.children()) {
        if (localName(child.name()) != "Relationship") {
            continue;
        }

        const std::string_view relationshipType = child.attribute("Type").as_string();
        if (relationshipType != "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet") {
            continue;
        }

        const std::string id = child.attribute("Id").as_string();
        const std::string target = child.attribute("Target").as_string();
        if (!id.empty() && !target.empty()) {
            targetsById[id] = resolveRelativeTarget(workbookPartPath, target);
        }
    }

    return targetsById;
}

} // namespace

OpenError Workbook::open(const std::filesystem::path& path) {
    clear();

    if (hasExtension(path, ".xls")) {
        m_lastError = OpenError::k_unsupported_legacy_xls;
        return m_lastError;
    }

    if (!hasExtension(path, ".xlsx")) {
        m_lastError = OpenError::k_not_xlsx_package;
        return m_lastError;
    }

    MINIDOCX_NAMESPACE::Zip archive;
    try {
        archive.open(path.string(), MINIDOCX_NAMESPACE::Zip::OpenMode::ReadOnly);
    } catch (...) {
        m_lastError = OpenError::k_not_xlsx_package;
        return m_lastError;
    }

    std::string contentTypesXml;
    std::string rootRelsXml;

    if (extractFileToString(archive, "/EncryptedPackage", contentTypesXml) ||
        extractFileToString(archive, "/EncryptionInfo", rootRelsXml)) {
        archive.close();
        m_lastError = OpenError::k_unsupported_encrypted;
        return m_lastError;
    }

    if (!extractFileToString(archive, "/[Content_Types].xml", contentTypesXml) ||
        !extractFileToString(archive, "/_rels/.rels", rootRelsXml)) {
        archive.close();
        m_lastError = OpenError::k_malformed_package;
        return m_lastError;
    }

    pugi::xml_document contentTypesDoc;
    pugi::xml_document rootRelsDoc;

    if (!contentTypesDoc.load_string(contentTypesXml.c_str()) || !rootRelsDoc.load_string(rootRelsXml.c_str())) {
        archive.close();
        m_lastError = OpenError::k_malformed_package;
        return m_lastError;
    }

    std::optional<std::string> workbookPart = findOfficeDocumentFromRootRels(rootRelsDoc);
    if (!workbookPart.has_value()) {
        workbookPart = findWorkbookPartFromContentTypes(contentTypesDoc);
    }

    if (!workbookPart.has_value()) {
        archive.close();
        m_lastError = OpenError::k_missing_workbook_part;
        return m_lastError;
    }

    std::string workbookXml;
    if (!extractFileToString(archive, *workbookPart, workbookXml)) {
        archive.close();
        m_lastError = OpenError::k_missing_workbook_part;
        return m_lastError;
    }

    std::string workbookRelsXml;
    const std::string workbookRelsEntry = workbookRelsPath(*workbookPart);
    if (!extractFileToString(archive, workbookRelsEntry, workbookRelsXml)) {
        archive.close();
        m_lastError = OpenError::k_malformed_package;
        return m_lastError;
    }

    pugi::xml_document workbookDoc;
    pugi::xml_document workbookRelsDoc;

    if (!workbookDoc.load_string(workbookXml.c_str()) || !workbookRelsDoc.load_string(workbookRelsXml.c_str())) {
        archive.close();
        m_lastError = OpenError::k_malformed_package;
        return m_lastError;
    }

    const auto targetsById = parseWorksheetTargets(workbookRelsDoc, *workbookPart);

    pugi::xml_node workbookNode;
    for (const pugi::xml_node child : workbookDoc.children()) {
        if (localName(child.name()) == "workbook") {
            workbookNode = child;
            break;
        }
    }

    if (!workbookNode) {
        archive.close();
        m_lastError = OpenError::k_malformed_package;
        return m_lastError;
    }

    pugi::xml_node sheetsNode;
    for (const pugi::xml_node child : workbookNode.children()) {
        if (localName(child.name()) == "sheets") {
            sheetsNode = child;
            break;
        }
    }

    if (!sheetsNode) {
        archive.close();
        m_lastError = OpenError::k_malformed_package;
        return m_lastError;
    }

    std::size_t index = 0;
    for (const pugi::xml_node child : sheetsNode.children()) {
        if (localName(child.name()) != "sheet") {
            continue;
        }

        const std::string name = child.attribute("name").as_string();
        const std::string relationshipId = child.attribute("r:id").as_string();

        if (name.empty() || relationshipId.empty()) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        auto targetIt = targetsById.find(relationshipId);
        if (targetIt == targetsById.end()) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        m_sheets.emplace_back(name, index, targetIt->second);
        ++index;
    }

    archive.close();
    m_isOpen = true;
    m_lastError = OpenError::k_none;
    return m_lastError;
}

bool Workbook::isOpen() const noexcept {
    return m_isOpen;
}

OpenError Workbook::lastError() const noexcept {
    return m_lastError;
}

std::size_t Workbook::sheetCount() const noexcept {
    return m_sheets.size();
}

const Worksheet* Workbook::sheetAt(std::size_t index) const noexcept {
    if (index >= m_sheets.size()) {
        return nullptr;
    }

    return &m_sheets[index];
}

void Workbook::clear() noexcept {
    m_isOpen = false;
    m_lastError = OpenError::k_none;
    m_sheets.clear();
}

} // namespace miniexcelx
