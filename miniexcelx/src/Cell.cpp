#include "miniexcelx/Cell.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace miniexcelx {

Cell::Cell(std::size_t row, std::size_t column, CellType type, std::string text, bool populated)
    : m_row(row),
      m_column(column),
      m_type(type),
      m_text(std::move(text)),
      m_isPopulated(populated) {
}

std::size_t Cell::row() const noexcept {
    return m_row;
}

std::size_t Cell::column() const noexcept {
    return m_column;
}

CellType Cell::type() const noexcept {
    return m_type;
}

std::string_view Cell::text() const noexcept {
    return m_text;
}

bool Cell::isPopulated() const noexcept {
    return m_isPopulated;
}

} // namespace miniexcelx
