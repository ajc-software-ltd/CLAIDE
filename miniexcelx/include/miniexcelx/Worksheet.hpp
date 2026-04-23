#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace miniexcelx {

class Worksheet {
public:
    Worksheet() = default;
    Worksheet(std::string name, std::size_t index, std::string partTarget);

    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::size_t index() const noexcept;
    [[nodiscard]] std::string_view partTarget() const noexcept;

private:
    std::string m_name{"Sheet1"};
    std::size_t m_index{0};
    std::string m_partTarget;
};

} // namespace miniexcelx
