#include "miniexcelx/MiniExcelX.hpp"

#include "minidocx/config.hpp"
#include "minidocx/utils/zip.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

bool writeArchive(const std::filesystem::path& path,
                  const std::initializer_list<std::pair<std::string, std::string>>& files) {
    try {
        MINIDOCX_NAMESPACE::Zip zip;
        zip.open(path.string(), MINIDOCX_NAMESPACE::Zip::OpenMode::Create);
        for (const auto& [entryName, content] : files) {
            zip.addFileFromString(entryName, content);
        }
        zip.close();
        return true;
    } catch (...) {
        return false;
    }
}

void writeTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    out << content;
}

bool runWorksheetParsingTest(const std::filesystem::path& tempDir) {
    static constexpr char k_contentTypes[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
  <Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>
  <Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
  <Override PartName="/xl/worksheets/sheet2.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
</Types>)xml";

    static constexpr char k_rootRels[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rIdWorkbook" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>)xml";

    static constexpr char k_workbook[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
  <sheets>
    <sheet name="Alpha" sheetId="1" r:id="rId1"/>
    <sheet name="Beta" sheetId="2" r:id="rId2"/>
  </sheets>
</workbook>)xml";

    static constexpr char k_workbookRels[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>
  <Relationship Id="rIdShared" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>)xml";

    static constexpr char k_sharedStrings[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="2" uniqueCount="2">
  <si><t>Hello</t></si>
  <si><t>World</t></si>
</sst>)xml";

    static constexpr char k_sheet1[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
  <sheetData>
    <row r="1">
      <c r="A1" t="s"><v>0</v></c>
      <c r="B1" t="inlineStr"><is><t>Inline</t></is></c>
      <c r="C1"><v>42.5</v></c>
      <c r="D1" t="b"><v>1</v></c>
      <c r="E1"/>
    </row>
    <row r="3">
      <c r="A3" t="s"><v>1</v></c>
      <c r="C3" t="e"><v>#N/A</v></c>
    </row>
  </sheetData>
</worksheet>)xml";

    static constexpr char k_sheet2[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
  <sheetData>
    <row r="1"><c r="A1"><f>SUM(1,2)</f><v>3</v></c></row>
  </sheetData>
</worksheet>)xml";

    const std::filesystem::path file = tempDir / "worksheet-types.xlsx";
    if (!writeArchive(file,
                      {
                          {"[Content_Types].xml", k_contentTypes},
                          {"_rels/.rels", k_rootRels},
                          {"xl/workbook.xml", k_workbook},
                          {"xl/_rels/workbook.xml.rels", k_workbookRels},
                          {"xl/sharedStrings.xml", k_sharedStrings},
                          {"xl/worksheets/sheet1.xml", k_sheet1},
                          {"xl/worksheets/sheet2.xml", k_sheet2},
                      })) {
        std::cerr << "failed to write worksheet parsing fixture\n";
        return false;
    }

    miniexcelx::Workbook workbook;
    if (workbook.open(file) != miniexcelx::OpenError::k_none || !workbook.isOpen()) {
        std::cerr << "worksheet parsing workbook did not open\n";
        return false;
    }

    if (workbook.sheetCount() != 2) {
        std::cerr << "sheet discovery count mismatch\n";
        return false;
    }

    const miniexcelx::Worksheet* alpha = workbook.sheetAt(0);
    const miniexcelx::Worksheet* beta = workbook.sheetAt(1);
    if (alpha == nullptr || beta == nullptr) {
        std::cerr << "sheetAt failed\n";
        return false;
    }

    if (alpha->name() != "Alpha" || beta->name() != "Beta") {
        std::cerr << "sheet order mismatch\n";
        return false;
    }

    if (alpha->cellCount() != 7 || alpha->nonEmptyCellCount() != 6 || alpha->rowCount() != 3 || alpha->columnCount() != 4) {
        std::cerr << "alpha worksheet counters mismatch\n";
        return false;
    }

    const miniexcelx::Cell* a1 = alpha->cellAt(0);
    const miniexcelx::Cell* b1 = alpha->cellAt(1);
    const miniexcelx::Cell* c1 = alpha->cellAt(2);
    const miniexcelx::Cell* d1 = alpha->cellAt(3);
    const miniexcelx::Cell* e1 = alpha->cellAt(4);
    const miniexcelx::Cell* a3 = alpha->cellAt(5);
    const miniexcelx::Cell* c3 = alpha->cellAt(6);

    if (!a1 || !b1 || !c1 || !d1 || !e1 || !a3 || !c3) {
        std::cerr << "alpha cell pointers mismatch\n";
        return false;
    }

    if (a1->type() != miniexcelx::CellType::k_string || a1->text() != "Hello" || a1->row() != 1 || a1->column() != 1) {
        std::cerr << "shared string cell parse mismatch\n";
        return false;
    }

    if (b1->type() != miniexcelx::CellType::k_inline_string || b1->text() != "Inline") {
        std::cerr << "inline string parse mismatch\n";
        return false;
    }

    if (c1->type() != miniexcelx::CellType::k_number || c1->text() != "42.5") {
        std::cerr << "number parse mismatch\n";
        return false;
    }

    if (d1->type() != miniexcelx::CellType::k_boolean || d1->text() != "1") {
        std::cerr << "boolean parse mismatch\n";
        return false;
    }

    if (e1->type() != miniexcelx::CellType::k_blank || e1->isPopulated()) {
        std::cerr << "blank handling mismatch\n";
        return false;
    }

    if (a3->type() != miniexcelx::CellType::k_string || a3->text() != "World") {
        std::cerr << "shared string index parse mismatch\n";
        return false;
    }

    if (c3->type() != miniexcelx::CellType::k_error || c3->text() != "#N/A") {
        std::cerr << "error cell parse mismatch\n";
        return false;
    }

    if (beta->cellCount() != 1 || beta->nonEmptyCellCount() != 1) {
        std::cerr << "beta worksheet counters mismatch\n";
        return false;
    }

    const miniexcelx::Cell* betaA1 = beta->cellAt(0);
    if (!betaA1 || betaA1->type() != miniexcelx::CellType::k_formula_cached || betaA1->text() != "3") {
        std::cerr << "formula cached shell parse mismatch\n";
        return false;
    }

    return true;
}

bool runMalformedPackageTest(const std::filesystem::path& tempDir) {
    const std::filesystem::path file = tempDir / "malformed.xlsx";
    if (!writeArchive(file, {{"_rels/.rels", "<Relationships/>"}})) {
        std::cerr << "failed to write malformed fixture\n";
        return false;
    }

    miniexcelx::Workbook workbook;
    if (workbook.open(file) != miniexcelx::OpenError::k_malformed_package) {
        std::cerr << "malformed package was not rejected\n";
        return false;
    }

    return true;
}

bool runLegacyXlsRejectionTest(const std::filesystem::path& tempDir) {
    const std::filesystem::path file = tempDir / "legacy.xls";
    writeTextFile(file, "legacy-xls-marker");

    miniexcelx::Workbook workbook;
    if (workbook.open(file) != miniexcelx::OpenError::k_unsupported_legacy_xls) {
        std::cerr << "legacy xls was not rejected\n";
        return false;
    }

    return true;
}

bool runEncryptedPackageRejectionTest(const std::filesystem::path& tempDir) {
    const std::filesystem::path file = tempDir / "encrypted.xlsx";
    if (!writeArchive(file,
                      {
                          {"[Content_Types].xml", "<Types/>"},
                          {"_rels/.rels", "<Relationships/>"},
                          {"EncryptionInfo", "enc"},
                          {"EncryptedPackage", "ciphertext"},
                      })) {
        std::cerr << "failed to write encrypted fixture\n";
        return false;
    }

    miniexcelx::Workbook workbook;
    if (workbook.open(file) != miniexcelx::OpenError::k_unsupported_encrypted) {
        std::cerr << "encrypted xlsx was not rejected\n";
        return false;
    }

    return true;
}

} // namespace

int main() {
    const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "miniexcelx_pr2_smoke";
    std::filesystem::remove_all(tempDir);
    std::filesystem::create_directories(tempDir);

    const bool ok = runWorksheetParsingTest(tempDir) && runMalformedPackageTest(tempDir) &&
                    runLegacyXlsRejectionTest(tempDir) && runEncryptedPackageRejectionTest(tempDir);

    std::filesystem::remove_all(tempDir);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
