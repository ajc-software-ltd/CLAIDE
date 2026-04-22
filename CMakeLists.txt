cmake_minimum_required(VERSION 3.20)
project(CLAIDE VERSION 0.0.130 LANGUAGES CXX)

set(CLAIDE_VERSION_STRING "0.0.130-dev")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan in debug builds" ON)

# Find dependencies
find_package(wxWidgets 3.2 REQUIRED COMPONENTS core base aui)
find_package(spdlog 1.11 REQUIRED)
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
find_package(Vulkan QUIET)

include(${wxWidgets_USE_FILE})

# Shader compilation
file(GLOB COMP_SHADERS src/vulkan/shaders/*.comp)
set(COMPILED_SHADERS "")
find_program(GLSLANG_VALIDATOR glslangValidator)

foreach(SHADER ${COMP_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(SPV_OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv)
    if(GLSLANG_VALIDATOR)
        add_custom_command(OUTPUT ${SPV_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
            COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER} -o ${SPV_OUTPUT}
            DEPENDS ${SHADER}
            COMMENT "Compiling shader ${SHADER_NAME}.comp")
    else()
        set(SPV_OUTPUT ${CMAKE_SOURCE_DIR}/src/vulkan/shaders/${SHADER_NAME}.spv)
    endif()
    list(APPEND COMPILED_SHADERS ${SPV_OUTPUT})
endforeach()

# Source files
set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/core/Encoding.cpp
    src/core/Document.cpp
    src/core/FileService.cpp
    src/core/DocumentWorkflowService.cpp
    src/core/MediaService.cpp
    src/core/FileBrowserService.cpp
    src/core/DocumentState.cpp
    src/core/TileCache.cpp
    src/core/MipmapGenerator.cpp
    src/core/CrashHandler.cpp
    src/core/VulkanRuntimeLoader.cpp
    src/core/runtime/VulkanRenderHost.cpp
    src/render/Canvas.cpp
    src/render/CanvasDocument.cpp
    src/render/CanvasView.cpp
    src/render/cpu/CPURenderer.cpp
    src/ui/Theme.cpp
    src/ui/ThumbnailCache.cpp
    src/ui/FileBrowserPanel.cpp
    src/ui/ActivityBar.cpp
    src/ui/SidebarPanel.cpp
    src/ui/PropertiesPanel.cpp
    src/ui/BackgroundPanel.cpp
    src/ui/PromptBar.cpp
    src/ui/CanvasPanel.cpp
    src/ui/EditorDocumentController.cpp
    src/ui/EditorPanel.cpp
    src/ui/ImageViewer.cpp
    src/ui/MainFrame.cpp
    src/platform/PlatformPaths.cpp
    src/vulkan/runtime/VulkanSwapchain.cpp
    src/vulkan/runtime/VulkanRenderer.cpp
)

set(HEADERS
    src/app/Application.hpp
    src/core/Encoding.hpp
    src/core/Document.hpp
    src/core/FileService.hpp
    src/core/DocumentWorkflowService.hpp
    src/core/MediaService.hpp
    src/core/FileBrowserService.hpp
    src/core/DocumentState.hpp
    src/core/TileCache.hpp
    src/core/MipmapGenerator.hpp
    src/core/CrashHandler.hpp
    src/core/VulkanRuntimeLoader.hpp
    src/core/runtime/RenderHost.hpp
    src/core/runtime/VulkanRenderHost.hpp
    src/render/Canvas.hpp
    src/render/CanvasDocument.hpp
    src/render/CanvasView.hpp
    src/render/cpu/CPURenderer.hpp
    src/core/media/AudioProvider.hpp
    src/core/media/VideoFrameProvider.hpp
    src/vulkan/runtime/VulkanSwapchain.hpp
    src/vulkan/runtime/VulkanRenderer.hpp
    src/ui/Theme.hpp
    src/ui/ThumbnailCache.hpp
    src/ui/FileBrowserPanel.hpp
    src/ui/ActivityBar.hpp
    src/ui/SidebarPanel.hpp
    src/ui/PropertiesPanel.hpp
    src/ui/BackgroundPanel.hpp
    src/ui/PromptBar.hpp
    src/ui/CanvasPanel.hpp
    src/ui/EditorDocumentController.hpp
    src/ui/EditorPanel.hpp
    src/ui/ImageViewer.hpp
    src/ui/MainFrame.hpp
    src/platform/PlatformPaths.hpp
)

add_executable(CLAIDE ${SOURCES} ${HEADERS})

set(VULKANAI_SOURCES
    src/vulkan/api/VulkanAI.cpp
    src/vulkan/filters/GPUEngine.cpp
    src/vulkan/runtime/VulkanContext.cpp
    src/vulkan/runtime/VulkanValidation.cpp
)

set(VULKANAI_HEADERS
    include/vulkanai/VulkanAI.h
    src/vulkan/filters/GPUEngine.hpp
    src/vulkan/runtime/VulkanContext.hpp
    src/vulkan/runtime/VulkanValidation.hpp
)

add_library(VulkanAI SHARED ${VULKANAI_SOURCES} ${VULKANAI_HEADERS})

if(WIN32)
    target_sources(CLAIDE PRIVATE resources.rc)
endif()

target_include_directories(CLAIDE PRIVATE src include ${ImageMagick_INCLUDE_DIRS})
target_compile_definitions(CLAIDE PRIVATE
    MAGICKCORE_QUANTUM_DEPTH=16
    MAGICKCORE_HDRI_ENABLE=1
    CLAIDE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    CLAIDE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    CLAIDE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    CLAIDE_VERSION_STRING="${CLAIDE_VERSION_STRING}"
)
target_link_libraries(CLAIDE PRIVATE ${wxWidgets_LIBRARIES} spdlog::spdlog ${ImageMagick_LIBRARIES})
target_link_libraries(CLAIDE PRIVATE VulkanAI)

target_include_directories(VulkanAI PRIVATE src include)
target_compile_definitions(VulkanAI PRIVATE
    VULKANAI_BUILD
    CLAIDE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    CLAIDE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    CLAIDE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)
target_link_libraries(VulkanAI PRIVATE spdlog::spdlog)
if(TARGET Vulkan::Vulkan)
    target_link_libraries(CLAIDE PRIVATE Vulkan::Vulkan)
    target_link_libraries(VulkanAI PRIVATE Vulkan::Vulkan)
    target_compile_definitions(VulkanAI PRIVATE VULKANAI_HAS_VULKAN=1)
    add_custom_target(shaders ALL DEPENDS ${COMPILED_SHADERS})
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CLAIDE PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(VulkanAI PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(CLAIDE PRIVATE /W4)
    target_compile_options(VulkanAI PRIVATE /W4)
endif()

# Sanitizers
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(CLAIDE PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(CLAIDE PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# Tests
enable_testing()
find_package(Catch2 3 QUIET)

if(Catch2_FOUND)
    set(TEST_SOURCES
        tests/EncodingTests.cpp
        tests/FileServiceTests.cpp
        tests/DocumentTests.cpp
        tests/MediaServiceTests.cpp
        tests/FileBrowserServiceTests.cpp
        tests/DocumentStateTests.cpp
        tests/TileCacheTests.cpp
        tests/MipmapTests.cpp
        tests/CanvasCoreTests.cpp
        tests/RendererFoundationTests.cpp
        tests/VulkanRuntimeFoundationTests.cpp
    )

    add_executable(CLAIDETests
        ${TEST_SOURCES}
        src/core/Encoding.cpp
        src/core/Document.cpp
        src/core/FileService.cpp
        src/core/DocumentWorkflowService.cpp
        src/core/MediaService.cpp
        src/core/FileBrowserService.cpp
        src/core/DocumentState.cpp
        src/core/TileCache.cpp
        src/core/MipmapGenerator.cpp
        src/core/VulkanRuntimeLoader.cpp
        src/core/runtime/VulkanRenderHost.cpp
        src/render/Canvas.cpp
        src/render/CanvasDocument.cpp
        src/render/CanvasView.cpp
        src/render/cpu/CPURenderer.cpp
        src/vulkan/runtime/VulkanSwapchain.cpp
        src/vulkan/runtime/VulkanRenderer.cpp
        src/platform/PlatformPaths.cpp
    )
    target_include_directories(CLAIDETests PRIVATE src include ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(CLAIDETests PRIVATE spdlog::spdlog Catch2::Catch2WithMain ${ImageMagick_LIBRARIES})
    target_compile_definitions(CLAIDETests PRIVATE
        MAGICKCORE_QUANTUM_DEPTH=16
        MAGICKCORE_HDRI_ENABLE=1
        CLAIDE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        CLAIDE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        CLAIDE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        CLAIDE_VERSION_STRING="${CLAIDE_VERSION_STRING}"
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(CLAIDETests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    include(Catch)
    catch_discover_tests(CLAIDETests)
endif()

# Format target
file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.hpp tests/*.cpp tests/*.hpp)

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting source files"
    )

    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking formatting"
    )
endif()

# Lint target
find_program(CLANG_TIDY
    NAMES clang-tidy-20 clang-tidy-19 clang-tidy-18 clang-tidy
    HINTS /usr/bin
)
if(CLANG_TIDY)
    add_custom_target(lint
        COMMAND ${CLANG_TIDY} --version
        COMMAND ${CMAKE_COMMAND} -E env
            CLANG_TIDY_BIN=${CLANG_TIDY}
            ${CMAKE_SOURCE_DIR}/scripts/run_lint.sh
            ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy"
    )

    add_custom_target(lint-fast
        COMMAND ${CLANG_TIDY} --version
        COMMAND ${CMAKE_COMMAND} -E env
            CLANG_TIDY_BIN=${CLANG_TIDY}
            ${CMAKE_SOURCE_DIR}/scripts/run_lint_changed.sh
            ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on changed files"
    )

    add_custom_target(lint-core
        COMMAND ${CLANG_TIDY} --version
        COMMAND ${CMAKE_COMMAND} -E env
            CLANG_TIDY_BIN=${CLANG_TIDY}
            LINT_SCOPE=core
            ${CMAKE_SOURCE_DIR}/scripts/run_lint.sh
            ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on core/platform translation units"
    )

    add_custom_target(lint-ui
        COMMAND ${CLANG_TIDY} --version
        COMMAND ${CMAKE_COMMAND} -E env
            CLANG_TIDY_BIN=${CLANG_TIDY}
            LINT_SCOPE=ui
            ${CMAKE_SOURCE_DIR}/scripts/run_lint.sh
            ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on UI translation units"
    )

    add_custom_target(lint-vulkan
        COMMAND ${CLANG_TIDY} --version
        COMMAND ${CMAKE_COMMAND} -E env
            CLANG_TIDY_BIN=${CLANG_TIDY}
            LINT_SCOPE=vulkan
            ${CMAKE_SOURCE_DIR}/scripts/run_lint.sh
            ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on Vulkan/render translation units"
    )

    add_custom_target(lint-tests
        COMMAND ${CLANG_TIDY} --version
        COMMAND ${CMAKE_COMMAND} -E env
            CLANG_TIDY_BIN=${CLANG_TIDY}
            LINT_SCOPE=tests
            ${CMAKE_SOURCE_DIR}/scripts/run_lint.sh
            ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on test translation units"
    )
endif()


if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(refresh-compile-commands
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Sync compile_commands.json to repository root for clangd/LSP"
    )
endif()

# Validate target
if(CLANG_FORMAT AND CLANG_TIDY AND Catch2_FOUND)
    add_custom_target(validate
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target format-check
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target lint
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running full validation"
    )
endif()

# Install rules
install(TARGETS CLAIDE VulkanAI
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
