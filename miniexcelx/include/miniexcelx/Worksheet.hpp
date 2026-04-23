#pragma once

#include "miniexcelx/Cell.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace miniexcelx {

class Worksheet {
public:
    Worksheet() = default;
    Worksheet(std::string name, std::size_t index, std::string partTarget);

    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::size_t index() const noexcept;
    [[nodiscard]] std::string_view partTarget() const noexcept;

    [[nodiscard]] std::size_t cellCount() const noexcept;
    [[nodiscard]] const Cell* cellAt(std::size_t index) const noexcept;
    [[nodiscard]] const std::vector<Cell>& cells() const noexcept;
    [[nodiscard]] std::size_t rowCount() const noexcept;
    [[nodiscard]] std::size_t columnCount() const noexcept;
    [[nodiscard]] std::size_t nonEmptyCellCount() const noexcept;

    void setParsedCells(std::vector<Cell> cells, std::size_t populatedRows, std::size_t populatedColumns, std::size_t nonEmptyCells);

private:
    std::string m_name{"Sheet1"};
    std::size_t m_index{0};
    std::string m_partTarget;
    std::vector<Cell> m_cells;
    std::size_t m_populatedRows{0};
    std::size_t m_populatedColumns{0};
    std::size_t m_nonEmptyCells{0};
};

} // namespace miniexcelx
