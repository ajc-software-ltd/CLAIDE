#include "miniexcelx/Worksheet.hpp"

#include <utility>

namespace miniexcelx {

Worksheet::Worksheet(std::string name) : m_name(std::move(name)) {
}

std::string_view Worksheet::name() const noexcept {
    return m_name;
}

std::size_t Worksheet::rowCount() const noexcept {
    return 0;
}

std::size_t Worksheet::columnCount() const noexcept {
    return 0;
}

} // namespace miniexcelx
