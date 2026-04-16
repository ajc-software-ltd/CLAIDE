#include <catch2/catch_test_macros.hpp>

#include "render/cpu/CPURenderer.hpp"
#include "vulkan/runtime/VulkanRenderer.hpp"
#include "vulkan/runtime/VulkanSwapchain.hpp"

using namespace Render;

TEST_CASE("VulkanSwapchain validates dimensions", "[renderer]") {
    VulkanSwapchain swapchain;
    auto invalid = swapchain.Create(0, 480);
    REQUIRE(!invalid.has_value());

    auto valid = swapchain.Create(640, 480);
    REQUIRE(valid.has_value());
    REQUIRE(swapchain.IsCreated());
}

TEST_CASE("VulkanRenderer requires created swapchain", "[renderer]") {
    VulkanSwapchain swapchain;
    VulkanRenderer renderer;

    auto initWithoutSwapchain = renderer.Initialize(&swapchain);
    REQUIRE(!initWithoutSwapchain.has_value());

    REQUIRE(swapchain.Create(800, 600).has_value());
    REQUIRE(renderer.Initialize(&swapchain).has_value());
    REQUIRE(renderer.RenderFrame().has_value());
}

TEST_CASE("CPURenderer tracks fallback status", "[renderer]") {
    CPURenderer renderer;
    renderer.SetStatus("CPU fallback active");
    REQUIRE(renderer.GetStatus() == "CPU fallback active");
}
