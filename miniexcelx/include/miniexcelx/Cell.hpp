#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace miniexcelx {

enum class CellType {
    k_blank,
    k_string,
    k_inline_string,
    k_number,
    k_boolean,
    k_error,
    k_formula_cached,
};

class Cell {
public:
    Cell() = default;
    Cell(std::size_t row, std::size_t column, CellType type, std::string text, bool populated);

    [[nodiscard]] std::size_t row() const noexcept;
    [[nodiscard]] std::size_t column() const noexcept;
    [[nodiscard]] CellType type() const noexcept;
    [[nodiscard]] std::string_view text() const noexcept;
    [[nodiscard]] bool isPopulated() const noexcept;

private:
    std::size_t m_row{0};
    std::size_t m_column{0};
    CellType m_type{CellType::k_blank};
    std::string m_text;
    bool m_isPopulated{false};
};

} // namespace miniexcelx
