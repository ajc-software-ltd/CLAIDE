#include "render/CanvasDocument.hpp"

namespace Render {

void CanvasDocument::SetContentType(CanvasContentType type) {
    m_contentType = type;
}

CanvasContentType CanvasDocument::GetContentType() const {
    return m_contentType;
}

void CanvasDocument::SetSource(std::string source) {
    m_source = std::move(source);
}

const std::string& CanvasDocument::GetSource() const {
    return m_source;
}

} // namespace Render
