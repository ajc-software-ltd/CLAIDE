cmake_minimum_required(VERSION 3.20)
project(CLIADE VERSION 0.0.3 LANGUAGES CXX)

set(CLIADE_VERSION_STRING "0.0.3-dev")

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
file(GLOB COMP_SHADERS src/gpu/shaders/*.comp)
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
        set(SPV_OUTPUT ${CMAKE_SOURCE_DIR}/src/gpu/shaders/${SHADER_NAME}.spv)
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
    src/core/MediaService.cpp
    src/core/DocumentState.cpp
    src/core/TileCache.cpp
    src/core/MipmapGenerator.cpp
    src/core/CrashHandler.cpp
    src/gpu/GPUEngine.cpp
    src/gpu/VulkanValidation.cpp
    src/render/vulkan/VulkanContext.cpp
    src/ui/Theme.cpp
    src/ui/ThumbnailCache.cpp
    src/ui/FileBrowserPanel.cpp
    src/ui/ActivityBar.cpp
    src/ui/SidebarPanel.cpp
    src/ui/PropertiesPanel.cpp
    src/ui/BackgroundPanel.cpp
    src/ui/PromptBar.cpp
    src/ui/EditorPanel.cpp
    src/ui/ImageViewer.cpp
    src/ui/MainFrame.cpp
    src/platform/PlatformPaths.cpp
)

set(HEADERS
    src/app/Application.hpp
    src/core/Encoding.hpp
    src/core/Document.hpp
    src/core/FileService.hpp
    src/core/MediaService.hpp
    src/core/DocumentState.hpp
    src/core/TileCache.hpp
    src/core/MipmapGenerator.hpp
    src/core/CrashHandler.hpp
    src/gpu/GPUEngine.hpp
    src/gpu/VulkanValidation.hpp
    src/ui/Theme.hpp
    src/ui/ThumbnailCache.hpp
    src/ui/FileBrowserPanel.hpp
    src/ui/ActivityBar.hpp
    src/ui/SidebarPanel.hpp
    src/ui/PropertiesPanel.hpp
    src/ui/BackgroundPanel.hpp
    src/ui/PromptBar.hpp
    src/ui/EditorPanel.hpp
    src/ui/ImageViewer.hpp
    src/ui/MainFrame.hpp
    src/platform/PlatformPaths.hpp
)

add_executable(CLIADE ${SOURCES} ${HEADERS})

if(WIN32)
    target_sources(CLIADE PRIVATE resources.rc)
endif()

target_include_directories(CLIADE PRIVATE src ${ImageMagick_INCLUDE_DIRS})
target_compile_definitions(CLIADE PRIVATE
    MAGICKCORE_QUANTUM_DEPTH=16
    MAGICKCORE_HDRI_ENABLE=1
    CLIADE_VERSION_STRING="${CLIADE_VERSION_STRING}"
)
target_link_libraries(CLIADE PRIVATE ${wxWidgets_LIBRARIES} spdlog::spdlog ${ImageMagick_Magick++_LIBRARIES})
if(TARGET Vulkan::Vulkan)
    target_link_libraries(CLIADE PRIVATE Vulkan::Vulkan)
    add_custom_target(shaders ALL DEPENDS ${COMPILED_SHADERS})
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CLIADE PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(CLIADE PRIVATE /W4)
endif()

# Sanitizers
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(CLIADE PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(CLIADE PRIVATE -fsanitize=address,undefined)
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
        tests/DocumentStateTests.cpp
        tests/TileCacheTests.cpp
        tests/MipmapTests.cpp
    )

    add_executable(CLIADETests ${TEST_SOURCES} src/core/Encoding.cpp src/core/Document.cpp src/core/FileService.cpp src/core/MediaService.cpp src/core/DocumentState.cpp src/core/TileCache.cpp src/core/MipmapGenerator.cpp src/platform/PlatformPaths.cpp)
    target_include_directories(CLIADETests PRIVATE src ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(CLIADETests PRIVATE spdlog::spdlog Catch2::Catch2WithMain ${ImageMagick_Magick++_LIBRARIES})
    target_compile_definitions(CLIADETests PRIVATE
        MAGICKCORE_QUANTUM_DEPTH=16
        MAGICKCORE_HDRI_ENABLE=1
        CLIADE_VERSION_STRING="${CLIADE_VERSION_STRING}"
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(CLIADETests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    include(Catch)
    catch_discover_tests(CLIADETests)
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
find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    add_custom_target(lint
        COMMAND ${CLANG_TIDY} ${ALL_SOURCE_FILES} -- -std=c++23 -I${CMAKE_SOURCE_DIR}/src
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy"
    )
endif()

# Validate target
if(CLANG_FORMAT AND CLANG_TIDY AND Catch2_FOUND)
    add_custom_target(validate
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target format-check
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target lint
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running full validation"
    )
endif()
