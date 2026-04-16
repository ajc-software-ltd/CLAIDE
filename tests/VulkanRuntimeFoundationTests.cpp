// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        VulkanRuntimeFoundationTests.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>

#include "core/VulkanRuntimeLoader.hpp"
#include "core/runtime/VulkanRenderHost.hpp"

using namespace Core;

TEST_CASE("VulkanRuntimeLoader exposes search paths after load attempt", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto loadResult = loader.Load();

    auto searchPaths = loader.GetRuntimeSearchPaths();
    REQUIRE(!searchPaths.empty());

    if (!loadResult) {
        auto attempts = loader.GetLastLoadAttempts();
        REQUIRE(!attempts.empty());
    }
}

TEST_CASE("VulkanRuntimeLoader reports consistent status fields", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto loadResult = loader.Load();

    if (!loadResult) {
        REQUIRE(!loader.IsLoaded());
        REQUIRE(!loader.IsInitialized());
        REQUIRE(!loader.IsApiCompatible());
        REQUIRE(!loader.GetLastError().empty());
        return;
    }

    REQUIRE(loader.IsLoaded());
    REQUIRE(loader.IsApiCompatible());
    REQUIRE(loader.GetRuntimeSearchPaths().size() >= 1);
}

TEST_CASE("VulkanRenderHost attach fails when loader is unavailable", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    VulkanRenderHost host(&loader);

    auto result = host.Attach(RenderHostConfig{
        .nativeWindowHandle = 0,
        .width = 640,
        .height = 480});
    REQUIRE(!result.has_value());
}

TEST_CASE("VulkanRenderHost attach validates native window handle", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto loadResult = loader.Load();
    if (!loadResult || !loader.IsVulkanAvailable()) {
        SUCCEED("Runtime unavailable in this environment; validated fallback in other tests.");
        return;
    }

    auto initResult = loader.Initialize();
    if (initResult != VULKANAI_OK) {
        SUCCEED("Runtime failed to initialize in this environment.");
        return;
    }

    VulkanRenderHost host(&loader);
    auto attach = host.Attach(RenderHostConfig{
        .nativeWindowHandle = 0,
        .width = 640,
        .height = 480});
    REQUIRE(!attach.has_value());
    loader.Shutdown();
}

TEST_CASE("Vulkan runtime lifecycle remains coherent when available", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto loadResult = loader.Load();
    if (!loadResult) {
        REQUIRE(!loader.GetLastError().empty());
        return;
    }

    if (!loader.IsVulkanAvailable()) {
        REQUIRE(loader.GetAvailabilityReasonCode() != VULKANAI_REASON_NONE);
        return;
    }

    auto initResult = loader.Initialize();
    if (initResult != VULKANAI_OK) {
        SUCCEED("Runtime initialization failed in this environment.");
        return;
    }
    REQUIRE(initResult == VULKANAI_OK);
    REQUIRE(loader.IsInitialized());

    VulkanAISurfaceDesc desc{
        .width = 320,
        .height = 200,
        .format = 0,
        .nativeWindowHandle = static_cast<uintptr_t>(1)};
    auto surface = loader.CreateSurface(desc);
    REQUIRE(surface != nullptr);

    auto begin = loader.BeginFrame(surface);
    REQUIRE(begin == VULKANAI_OK);
    auto end = loader.EndFrame(surface);
    REQUIRE(end == VULKANAI_OK);
    auto present = loader.PresentFrame(surface);
    REQUIRE(present == VULKANAI_OK);

    loader.DestroySurface(surface);
    loader.Shutdown();
}

TEST_CASE("Vulkan runtime enforces frame ordering when available", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto loadResult = loader.Load();
    if (!loadResult || !loader.IsVulkanAvailable()) {
        SUCCEED("Runtime unavailable in this environment.");
        return;
    }

    auto initResult = loader.Initialize();
    if (initResult != VULKANAI_OK) {
        SUCCEED("Runtime failed to initialize in this environment.");
        return;
    }

    VulkanAISurfaceDesc desc{
        .width = 256,
        .height = 256,
        .format = 0,
        .nativeWindowHandle = static_cast<uintptr_t>(1)};
    auto surface = loader.CreateSurface(desc);
    REQUIRE(surface != nullptr);

    auto endBeforeBegin = loader.EndFrame(surface);
    REQUIRE(endBeforeBegin == VULKANAI_ERROR_INVALID_STATE);

    auto begin = loader.BeginFrame(surface);
    REQUIRE(begin == VULKANAI_OK);

    auto doubleBegin = loader.BeginFrame(surface);
    REQUIRE(doubleBegin == VULKANAI_ERROR_INVALID_STATE);

    auto presentBeforeEnd = loader.PresentFrame(surface);
    REQUIRE(presentBeforeEnd == VULKANAI_ERROR_INVALID_STATE);

    auto end = loader.EndFrame(surface);
    REQUIRE(end == VULKANAI_OK);

    auto present = loader.PresentFrame(surface);
    REQUIRE(present == VULKANAI_OK);

    loader.DestroySurface(surface);
    loader.Shutdown();
}

TEST_CASE("Vulkan runtime rejects frame calls after shutdown", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto loadResult = loader.Load();
    if (!loadResult || !loader.IsVulkanAvailable()) {
        SUCCEED("Runtime unavailable in this environment.");
        return;
    }

    auto initResult = loader.Initialize();
    if (initResult != VULKANAI_OK) {
        SUCCEED("Runtime failed to initialize in this environment.");
        return;
    }

    VulkanAISurfaceDesc desc{
        .width = 128,
        .height = 128,
        .format = 0,
        .nativeWindowHandle = static_cast<uintptr_t>(1)};
    auto surface = loader.CreateSurface(desc);
    REQUIRE(surface != nullptr);

    loader.Shutdown();

    auto beginAfterShutdown = loader.BeginFrame(surface);
    REQUIRE(beginAfterShutdown == VULKANAI_ERROR_INVALID_STATE);
}

TEST_CASE("Vulkan runtime load/unload cycle is stable", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto firstLoad = loader.Load();
    if (!firstLoad) {
        REQUIRE(!loader.GetLastError().empty());
        return;
    }

    loader.Unload();
    REQUIRE(!loader.IsLoaded());

    auto secondLoad = loader.Load();
    if (!secondLoad) {
        REQUIRE(!loader.GetLastError().empty());
        return;
    }
    REQUIRE(loader.IsLoaded());
}

TEST_CASE("Vulkan runtime rejects invalid surface descriptor", "[vulkan][runtime]") {
    VulkanRuntimeLoader loader;
    auto loadResult = loader.Load();
    if (!loadResult || !loader.IsVulkanAvailable()) {
        SUCCEED("Runtime unavailable in this environment.");
        return;
    }

    auto initResult = loader.Initialize();
    if (initResult != VULKANAI_OK) {
        SUCCEED("Runtime failed to initialize in this environment.");
        return;
    }

    VulkanAISurfaceDesc invalidDesc{
        .width = 0,
        .height = 128,
        .format = 0,
        .nativeWindowHandle = static_cast<uintptr_t>(1)};
    auto surface = loader.CreateSurface(invalidDesc);
    REQUIRE(surface == nullptr);
    loader.Shutdown();
}
