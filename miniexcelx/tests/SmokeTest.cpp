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

bool runValidXlsxTest(const std::filesystem::path& tempDir) {
    static constexpr char k_contentTypes[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
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
</Relationships>)xml";

    const std::filesystem::path file = tempDir / "valid.xlsx";
    if (!writeArchive(file,
                      {
                          {"[Content_Types].xml", k_contentTypes},
                          {"_rels/.rels", k_rootRels},
                          {"xl/workbook.xml", k_workbook},
                          {"xl/_rels/workbook.xml.rels", k_workbookRels},
                          {"xl/worksheets/sheet1.xml", "<worksheet/>"},
                          {"xl/worksheets/sheet2.xml", "<worksheet/>"},
                      })) {
        std::cerr << "failed to write valid xlsx fixture\n";
        return false;
    }

    miniexcelx::Workbook workbook;
    if (workbook.open(file) != miniexcelx::OpenError::k_none || !workbook.isOpen()) {
        std::cerr << "valid xlsx did not open; error=" << static_cast<int>(workbook.lastError()) << "\n";
        return false;
    }

    if (workbook.sheetCount() != 2) {
        std::cerr << "sheet count mismatch\n";
        return false;
    }

    const miniexcelx::Worksheet* first = workbook.sheetAt(0);
    const miniexcelx::Worksheet* second = workbook.sheetAt(1);
    if (first == nullptr || second == nullptr) {
        std::cerr << "sheetAt returned null\n";
        return false;
    }

    if (first->name() != "Alpha" || second->name() != "Beta") {
        std::cerr << "sheet names mismatch\n";
        return false;
    }

    if (first->index() != 0 || second->index() != 1) {
        std::cerr << "sheet indexes mismatch\n";
        return false;
    }

    if (first->partTarget() != "/xl/worksheets/sheet1.xml" || second->partTarget() != "/xl/worksheets/sheet2.xml") {
        std::cerr << "sheet part targets mismatch\n";
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
    const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "miniexcelx_pr1_smoke";
    std::filesystem::remove_all(tempDir);
    std::filesystem::create_directories(tempDir);

    const bool ok = runValidXlsxTest(tempDir) && runMalformedPackageTest(tempDir) &&
                    runLegacyXlsRejectionTest(tempDir) && runEncryptedPackageRejectionTest(tempDir);

    std::filesystem::remove_all(tempDir);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
