#include <catch2/catch_test_macros.hpp>

#include "render/Canvas.hpp"

using namespace Render;

TEST_CASE("CanvasDocument stores content metadata", "[canvas]") {
    CanvasDocument document;
    document.SetContentType(CanvasContentType::Image);
    document.SetSource("/tmp/example.png");

    REQUIRE(document.GetContentType() == CanvasContentType::Image);
    REQUIRE(document.GetSource() == "/tmp/example.png");
}

TEST_CASE("CanvasView zoom is clamped and pan is tracked", "[canvas]") {
    CanvasView view;
    view.SetZoom(0.01F);
    view.SetPan(42.0F, -7.0F);

    REQUIRE(view.GetZoom() == 0.1F);
    REQUIRE(view.GetPanX() == 42.0F);
    REQUIRE(view.GetPanY() == -7.0F);
}

TEST_CASE("Canvas runtime status is mutable", "[canvas]") {
    Canvas canvas;
    canvas.SetRuntimeReady(true, "Runtime OK");

    REQUIRE(canvas.IsRuntimeReady());
    REQUIRE(canvas.GetRuntimeStatus() == "Runtime OK");
}
