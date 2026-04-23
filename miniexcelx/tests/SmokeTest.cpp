#include "miniexcelx/MiniExcelX.hpp"

#include <cstdlib>

int main() {
    const miniexcelx::Workbook workbook;
    const miniexcelx::Worksheet worksheet;
    const miniexcelx::Cell cell;

    if (workbook.sheetCount() != 0) {
        return EXIT_FAILURE;
    }

    if (worksheet.rowCount() != 0 || worksheet.columnCount() != 0) {
        return EXIT_FAILURE;
    }

    if (!cell.text().empty()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
