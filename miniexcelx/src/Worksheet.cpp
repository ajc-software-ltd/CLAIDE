#include "miniexcelx/Worksheet.hpp"

#include <utility>
#include <vector>

namespace miniexcelx {

Worksheet::Worksheet(std::string name, std::size_t index, std::string partTarget)
    : m_name(std::move(name)),
      m_index(index),
      m_partTarget(std::move(partTarget)) {
}

std::string_view Worksheet::name() const noexcept {
    return m_name;
}

std::size_t Worksheet::index() const noexcept {
    return m_index;
}

std::string_view Worksheet::partTarget() const noexcept {
    return m_partTarget;
}

std::size_t Worksheet::cellCount() const noexcept {
    return m_cells.size();
}

const Cell* Worksheet::cellAt(std::size_t index) const noexcept {
    if (index >= m_cells.size()) {
        return nullptr;
    }

    return &m_cells[index];
}

const std::vector<Cell>& Worksheet::cells() const noexcept {
    return m_cells;
}

std::size_t Worksheet::rowCount() const noexcept {
    return m_populatedRows;
}

std::size_t Worksheet::columnCount() const noexcept {
    return m_populatedColumns;
}

std::size_t Worksheet::nonEmptyCellCount() const noexcept {
    return m_nonEmptyCells;
}

void Worksheet::setParsedCells(std::vector<Cell> cells,
                               std::size_t populatedRows,
                               std::size_t populatedColumns,
                               std::size_t nonEmptyCells) {
    m_cells = std::move(cells);
    m_populatedRows = populatedRows;
    m_populatedColumns = populatedColumns;
    m_nonEmptyCells = nonEmptyCells;
}

} // namespace miniexcelx
