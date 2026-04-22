#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "render/Canvas.hpp"
#include "render/CanvasRegistry.hpp"

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

TEST_CASE("CanvasRegistry tracks multi-canvas lifecycle", "[canvas]") {
    CanvasRegistry registry;
    auto firstId = registry.CreateCanvas();
    auto secondId = registry.CreateCanvas();

    REQUIRE(firstId != secondId);
    REQUIRE(registry.Count() == 2);
    REQUIRE(registry.GetCanvas(firstId).has_value());

    REQUIRE(registry.RemoveCanvas(firstId));
    REQUIRE(registry.Count() == 1);
    REQUIRE(!registry.GetCanvas(firstId).has_value());
}

TEST_CASE("CanvasRegistry validates and sorts PiP composition layers", "[canvas]") {
    CanvasRegistry registry;
    auto backId = registry.CreateCanvas();
    auto frontId = registry.CreateCanvas();

    std::vector<CanvasLayer> layers{{.canvasId = frontId, .x = 200.0F, .y = 120.0F, .width = 220.0F, .height = 140.0F, .zIndex = 2},
                                    {.canvasId = backId, .x = 0.0F, .y = 0.0F, .width = 640.0F, .height = 360.0F, .zIndex = 1}};

    REQUIRE(registry.SetComposition(layers));
    REQUIRE(registry.GetComposition().size() == 2);
    REQUIRE(registry.GetComposition().front().canvasId == backId);
    REQUIRE(registry.GetComposition().back().canvasId == frontId);
}
