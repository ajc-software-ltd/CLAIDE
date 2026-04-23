#include "miniexcelx/Worksheet.hpp"

#include <utility>

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

} // namespace miniexcelx
