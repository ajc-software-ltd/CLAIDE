#pragma once

#include <string>
#include <string_view>

namespace miniexcelx {

class Cell {
public:
    Cell() = default;

    [[nodiscard]] std::string_view text() const noexcept;

private:
    std::string m_text;
};

} // namespace miniexcelx
