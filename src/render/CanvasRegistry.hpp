#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "render/CanvasDocument.hpp"

namespace Render {

struct CanvasLayer
{
    uint32_t canvasId{0};
    float x{0.0F};
    float y{0.0F};
    float width{0.0F};
    float height{0.0F};
    int zIndex{0};
};

class CanvasRegistry
{
  public:
    uint32_t CreateCanvas(CanvasDocument document = {});
    bool RemoveCanvas(uint32_t canvasId);
    std::optional<CanvasDocument> GetCanvas(uint32_t canvasId) const;
    std::size_t Count() const;

    bool SetComposition(std::vector<CanvasLayer> layers);
    const std::vector<CanvasLayer>& GetComposition() const;

  private:
    uint32_t m_nextCanvasId{1};
    std::unordered_map<uint32_t, CanvasDocument> m_canvases;
    std::vector<CanvasLayer> m_layers;
};

} // namespace Render
