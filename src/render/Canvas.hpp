#pragma once

#include <string>

#include "render/CanvasDocument.hpp"
#include "render/CanvasView.hpp"

namespace Render {

class Canvas
{
  public:
    CanvasDocument& Document();
    const CanvasDocument& Document() const;

    CanvasView& View();
    const CanvasView& View() const;

    void SetRuntimeReady(bool ready, std::string status);
    bool IsRuntimeReady() const;
    const std::string& GetRuntimeStatus() const;

  private:
    CanvasDocument m_document;
    CanvasView m_view;
    bool m_runtimeReady{false};
    std::string m_runtimeStatus{"Runtime not initialized"};
};

} // namespace Render
