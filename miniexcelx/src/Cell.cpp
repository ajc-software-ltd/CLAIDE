#include "miniexcelx/Cell.hpp"

#include <string_view>

namespace miniexcelx {

std::string_view Cell::text() const noexcept {
    return m_text;
}

} // namespace miniexcelx
