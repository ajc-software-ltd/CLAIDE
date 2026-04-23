#pragma once

#include "miniexcelx/Worksheet.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

namespace miniexcelx {

enum class OpenError {
    k_none,
    k_unsupported_legacy_xls,
    k_not_xlsx_package,
    k_unsupported_encrypted,
    k_malformed_package,
    k_missing_workbook_part,
};

class Workbook {
public:
    Workbook() = default;

    [[nodiscard]] OpenError open(const std::filesystem::path& path);
    [[nodiscard]] bool isOpen() const noexcept;
    [[nodiscard]] OpenError lastError() const noexcept;
    [[nodiscard]] std::size_t sheetCount() const noexcept;
    [[nodiscard]] const Worksheet* sheetAt(std::size_t index) const noexcept;

private:
    void clear() noexcept;

    bool m_isOpen{false};
    OpenError m_lastError{OpenError::k_none};
    std::vector<Worksheet> m_sheets;
};

} // namespace miniexcelx
