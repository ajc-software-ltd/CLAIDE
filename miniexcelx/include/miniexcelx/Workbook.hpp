#pragma once

#include <cstddef>

namespace miniexcelx {

class Workbook {
public:
    Workbook() = default;

    [[nodiscard]] std::size_t sheetCount() const noexcept;
};

} // namespace miniexcelx
