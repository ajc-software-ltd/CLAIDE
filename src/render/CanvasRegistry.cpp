#include "render/CanvasRegistry.hpp"

#include <algorithm>

namespace Render {

uint32_t CanvasRegistry::CreateCanvas(CanvasDocument document) {
    const uint32_t canvasId = m_nextCanvasId++;
    m_canvases.emplace(canvasId, std::move(document));
    return canvasId;
}

bool CanvasRegistry::RemoveCanvas(uint32_t canvasId) {
    if (m_canvases.erase(canvasId) == 0) {
        return false;
    }

    m_layers.erase(std::remove_if(m_layers.begin(), m_layers.end(), [canvasId](const CanvasLayer& layer) {
                       return layer.canvasId == canvasId;
                   }),
                   m_layers.end());
    return true;
}

std::optional<CanvasDocument> CanvasRegistry::GetCanvas(uint32_t canvasId) const {
    const auto it = m_canvases.find(canvasId);
    if (it == m_canvases.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::size_t CanvasRegistry::Count() const {
    return m_canvases.size();
}

bool CanvasRegistry::SetComposition(std::vector<CanvasLayer> layers) {
    const bool allKnown = std::all_of(layers.begin(), layers.end(), [this](const CanvasLayer& layer) {
        return m_canvases.contains(layer.canvasId) && layer.width > 0.0F && layer.height > 0.0F;
    });

    if (!allKnown) {
        return false;
    }

    std::sort(layers.begin(), layers.end(), [](const CanvasLayer& lhs, const CanvasLayer& rhs) {
        return lhs.zIndex < rhs.zIndex;
    });
    m_layers = std::move(layers);
    return true;
}

const std::vector<CanvasLayer>& CanvasRegistry::GetComposition() const {
    return m_layers;
}

} // namespace Render
