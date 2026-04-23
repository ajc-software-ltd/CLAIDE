#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace miniexcelx {

class Worksheet {
public:
    Worksheet() = default;
    explicit Worksheet(std::string name);

    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::size_t rowCount() const noexcept;
    [[nodiscard]] std::size_t columnCount() const noexcept;

private:
    std::string m_name{"Sheet1"};
};

} // namespace miniexcelx
