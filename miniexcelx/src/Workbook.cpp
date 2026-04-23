#include "miniexcelx/Workbook.hpp"

#include "minidocx/config.hpp"
#include "minidocx/utils/zip.hpp"
#include "pugixml.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

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

[[nodiscard]] bool extractFileToString(MINIDOCX_NAMESPACE::Zip& archive,
                                       std::string_view canonicalPath,
                                       std::string& outContent) {
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

struct WorkbookRelationships {
    std::unordered_map<std::string, std::string> worksheetTargetsById;
    std::optional<std::string> sharedStringsPart;
};

[[nodiscard]] WorkbookRelationships parseWorkbookRelationships(const pugi::xml_document& workbookRelsDoc,
                                                              std::string_view workbookPartPath) {
    WorkbookRelationships relationships;

    const pugi::xml_node root = workbookRelsDoc.document_element();
    for (const pugi::xml_node child : root.children()) {
        if (localName(child.name()) != "Relationship") {
            continue;
        }

        const std::string id = child.attribute("Id").as_string();
        const std::string target = child.attribute("Target").as_string();
        const std::string type = child.attribute("Type").as_string();

        if (id.empty() || target.empty() || type.empty()) {
            continue;
        }

        const std::string resolvedTarget = resolveRelativeTarget(workbookPartPath, target);
        if (type == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet") {
            relationships.worksheetTargetsById[id] = resolvedTarget;
        } else if (type == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings") {
            relationships.sharedStringsPart = resolvedTarget;
        }
    }

    return relationships;
}

[[nodiscard]] std::string nodeText(const pugi::xml_node& node) {
    std::string text;
    if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata) {
        text.append(node.value());
    }

    for (const pugi::xml_node child : node.children()) {
        text.append(nodeText(child));
    }

    return text;
}

[[nodiscard]] std::vector<std::string> parseSharedStrings(const pugi::xml_document& sharedStringsDoc) {
    std::vector<std::string> table;
    const pugi::xml_node root = sharedStringsDoc.document_element();
    for (const pugi::xml_node item : root.children()) {
        if (localName(item.name()) != "si") {
            continue;
        }

        std::string resolved;
        for (const pugi::xml_node child : item.children()) {
            const std::string_view node = localName(child.name());
            if (node == "t" || node == "r") {
                resolved.append(nodeText(child));
            }
        }

        table.push_back(resolved);
    }

    return table;
}

[[nodiscard]] bool parseCellReference(std::string_view reference, std::size_t& row, std::size_t& column) {
    std::size_t index = 0;
    std::size_t parsedColumn = 0;
    while (index < reference.size() && std::isalpha(static_cast<unsigned char>(reference[index]))) {
        parsedColumn = (parsedColumn * 26) + (std::toupper(static_cast<unsigned char>(reference[index])) - 'A' + 1);
        ++index;
    }

    if (parsedColumn == 0 || index >= reference.size()) {
        return false;
    }

    std::size_t parsedRow = 0;
    while (index < reference.size() && std::isdigit(static_cast<unsigned char>(reference[index]))) {
        parsedRow = (parsedRow * 10) + static_cast<std::size_t>(reference[index] - '0');
        ++index;
    }

    if (parsedRow == 0 || index != reference.size()) {
        return false;
    }

    row = parsedRow;
    column = parsedColumn;
    return true;
}

struct ParsedWorksheetData {
    std::vector<Cell> cells;
    std::size_t populatedRows{0};
    std::size_t populatedColumns{0};
    std::size_t nonEmptyCells{0};
    bool valid{true};
};

[[nodiscard]] ParsedWorksheetData parseWorksheetData(const pugi::xml_document& worksheetDoc,
                                                     const std::vector<std::string>& sharedStringsTable) {
    ParsedWorksheetData result;

    pugi::xml_node worksheetNode = worksheetDoc.document_element();
    if (!worksheetNode || localName(worksheetNode.name()) != "worksheet") {
        result.valid = false;
        return result;
    }

    pugi::xml_node sheetData;
    for (const pugi::xml_node child : worksheetNode.children()) {
        if (localName(child.name()) == "sheetData") {
            sheetData = child;
            break;
        }
    }

    if (!sheetData) {
        return result;
    }

    std::size_t rowIndexFromOrder = 1;
    for (const pugi::xml_node rowNode : sheetData.children()) {
        if (localName(rowNode.name()) != "row") {
            continue;
        }

        std::size_t rowIndex = rowNode.attribute("r").as_ullong();
        if (rowIndex == 0) {
            rowIndex = rowIndexFromOrder;
        }

        std::size_t columnFromOrder = 1;
        for (const pugi::xml_node cellNode : rowNode.children()) {
            if (localName(cellNode.name()) != "c") {
                continue;
            }

            std::size_t cellRow = rowIndex;
            std::size_t cellColumn = columnFromOrder;

            const std::string reference = cellNode.attribute("r").as_string();
            if (!reference.empty()) {
                const bool parsedReference = parseCellReference(reference, cellRow, cellColumn);
                (void)parsedReference;
            }

            const std::string typeToken = cellNode.attribute("t").as_string();
            const bool hasFormula = static_cast<bool>(cellNode.child("f"));
            std::string textValue;
            CellType cellType = CellType::k_blank;
            bool populated = false;

            if (typeToken == "s") {
                const std::string indexText = cellNode.child("v").text().as_string();
                if (!indexText.empty()) {
                    const std::size_t sharedIndex = static_cast<std::size_t>(std::stoull(indexText));
                    if (sharedIndex < sharedStringsTable.size()) {
                        textValue = sharedStringsTable[sharedIndex];
                        cellType = CellType::k_string;
                        populated = true;
                    } else {
                        result.valid = false;
                        return result;
                    }
                }
            } else if (typeToken == "inlineStr") {
                const pugi::xml_node inlineNode = cellNode.child("is");
                textValue = nodeText(inlineNode);
                cellType = CellType::k_inline_string;
                populated = !textValue.empty();
            } else if (typeToken == "b") {
                textValue = cellNode.child("v").text().as_string();
                cellType = CellType::k_boolean;
                populated = !textValue.empty();
            } else if (typeToken == "e") {
                textValue = cellNode.child("v").text().as_string();
                cellType = CellType::k_error;
                populated = !textValue.empty();
            } else {
                textValue = cellNode.child("v").text().as_string();
                if (hasFormula) {
                    cellType = CellType::k_formula_cached;
                    populated = !textValue.empty();
                } else if (!textValue.empty()) {
                    cellType = CellType::k_number;
                    populated = true;
                }
            }

            result.cells.emplace_back(cellRow, cellColumn, cellType, textValue, populated);

            if (populated) {
                result.nonEmptyCells++;
                result.populatedRows = std::max(result.populatedRows, cellRow);
                result.populatedColumns = std::max(result.populatedColumns, cellColumn);
            }

            columnFromOrder = std::max(columnFromOrder, cellColumn + 1);
        }

        rowIndexFromOrder = std::max(rowIndexFromOrder, rowIndex + 1);
    }

    return result;
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

    const WorkbookRelationships workbookRelationships = parseWorkbookRelationships(workbookRelsDoc, *workbookPart);

    std::vector<std::string> sharedStringsTable;
    if (workbookRelationships.sharedStringsPart.has_value()) {
        std::string sharedStringsXml;
        if (!extractFileToString(archive, *workbookRelationships.sharedStringsPart, sharedStringsXml)) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        pugi::xml_document sharedStringsDoc;
        if (!sharedStringsDoc.load_string(sharedStringsXml.c_str())) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        sharedStringsTable = parseSharedStrings(sharedStringsDoc);
    }

    pugi::xml_node workbookNode = workbookDoc.document_element();
    if (!workbookNode || localName(workbookNode.name()) != "workbook") {
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

        auto targetIt = workbookRelationships.worksheetTargetsById.find(relationshipId);
        if (targetIt == workbookRelationships.worksheetTargetsById.end()) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        Worksheet sheet(name, index, targetIt->second);

        std::string worksheetXml;
        if (!extractFileToString(archive, targetIt->second, worksheetXml)) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        pugi::xml_document worksheetDoc;
        if (!worksheetDoc.load_string(worksheetXml.c_str())) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        ParsedWorksheetData parsed = parseWorksheetData(worksheetDoc, sharedStringsTable);
        if (!parsed.valid) {
            archive.close();
            m_lastError = OpenError::k_malformed_package;
            return m_lastError;
        }

        sheet.setParsedCells(std::move(parsed.cells), parsed.populatedRows, parsed.populatedColumns, parsed.nonEmptyCells);
        m_sheets.push_back(std::move(sheet));
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
