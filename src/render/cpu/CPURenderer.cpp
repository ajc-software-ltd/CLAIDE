#include "render/cpu/CPURenderer.hpp"

#include <utility>

namespace Render {

void CPURenderer::SetStatus(std::string status) {
    m_status = std::move(status);
}

const std::string& CPURenderer::GetStatus() const {
    return m_status;
}

} // namespace Render
