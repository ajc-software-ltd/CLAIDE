#include <array>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "render/cpu/CPURenderer.hpp"
#include "vulkan/runtime/VulkanRenderer.hpp"
#include "vulkan/runtime/VulkanSwapchain.hpp"
#include "vulkan/runtime/VulkanDebug.hpp"
#include "vulkan/runtime/VulkanText.hpp"
#include "vulkan/runtime/VulkanTexture.hpp"

using namespace Render;

TEST_CASE("VulkanSwapchain validates dimensions", "[renderer]") {
    VulkanSwapchain swapchain;
    auto invalid = swapchain.Create(0, 480);
    REQUIRE(!invalid.has_value());

    auto valid = swapchain.Create(640, 480);
    REQUIRE(valid.has_value());
    REQUIRE(swapchain.IsCreated());
    REQUIRE(swapchain.GetWidth() == 640);
    REQUIRE(swapchain.GetHeight() == 480);

    auto recreate = swapchain.Recreate(1024, 768);
    REQUIRE(recreate.has_value());
    REQUIRE(swapchain.GetWidth() == 1024);
    REQUIRE(swapchain.GetHeight() == 768);
}

TEST_CASE("VulkanRenderer requires created swapchain", "[renderer]") {
    VulkanSwapchain swapchain;
    VulkanRenderer renderer;

    auto initWithoutSwapchain = renderer.Initialize(&swapchain);
    REQUIRE(!initWithoutSwapchain.has_value());

    REQUIRE(swapchain.Create(800, 600).has_value());
    REQUIRE(renderer.Initialize(&swapchain).has_value());
    REQUIRE(renderer.IsInitialized());

    REQUIRE(renderer.BeginFrame().has_value());
    REQUIRE(renderer.IsFrameActive());
    auto duplicateBegin = renderer.BeginFrame();
    REQUIRE(!duplicateBegin.has_value());

    REQUIRE(renderer.EndFrame().has_value());
    REQUIRE(!renderer.IsFrameActive());
    REQUIRE(renderer.Present().has_value());
    REQUIRE(renderer.RenderFrame().has_value());

    renderer.Shutdown();
    REQUIRE(!renderer.IsInitialized());
}

TEST_CASE("CPURenderer tracks fallback status", "[renderer]") {
    CPURenderer renderer;
    renderer.SetStatus("CPU fallback active");
    REQUIRE(renderer.GetStatus() == "CPU fallback active");
}

TEST_CASE("VulkanTexture validates upload contract", "[renderer]") {
    VulkanTexture texture;

    auto invalid = texture.UploadRgba8(nullptr, 0, 0, 0);
    REQUIRE(!invalid.has_value());

    std::array<std::uint8_t, 16> rgba{};
    auto valid = texture.UploadRgba8(rgba.data(), rgba.size(), 2, 2);
    REQUIRE(valid.has_value());
    REQUIRE(texture.IsReady());
    REQUIRE(texture.GetWidth() == 2);
    REQUIRE(texture.GetHeight() == 2);
}

TEST_CASE("VulkanText validates atlas build input", "[renderer]") {
    VulkanText text;

    auto missingFont = text.BuildGlyphAtlas("", 14.0F);
    REQUIRE(!missingFont.has_value());

    auto invalidSize = text.BuildGlyphAtlas("Monospace", 0.0F);
    REQUIRE(!invalidSize.has_value());

    auto valid = text.BuildGlyphAtlas("Monospace", 14.0F);
    REQUIRE(valid.has_value());
    REQUIRE(text.HasAtlas());
    REQUIRE(text.GetFontFace() == "Monospace");
    REQUIRE(text.GetPointSize() == 14.0F);
}

TEST_CASE("VulkanDebug tracks validation state and error messages", "[renderer]") {
    VulkanDebug debug;
    REQUIRE(!debug.IsValidationEnabled());
    REQUIRE(!debug.HasErrors());

    debug.EnableValidation(true);
    REQUIRE(debug.IsValidationEnabled());

    debug.AddMessage("warning: swapchain recreate");
    REQUIRE(!debug.HasErrors());

    debug.AddMessage("error: vkCreateInstance failed");
    REQUIRE(debug.HasErrors());
}
