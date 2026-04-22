
cmake_minimum_required(VERSION 3.28)
project(minidocx VERSION 1.0.0 LANGUAGES C CXX)

option(BUILD_SHARED    "Build shared library"         OFF)
option(BUILD_EXAMPLES  "Build the examples"           ON)
option(BUILD_TESTS     "Build the unit tests"         OFF)
option(WITH_STATIC_CRT "Use static C Runtime library" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}") # Added in version 3.27.
set(CMAKE_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}") # Added in version 4.0.

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/exe")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/dll")

set(THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/3rdparty" CACHE PATH "3rdparty")
set(MINIZ_DIR "${THIRD_PARTY_DIR}/miniz-3.0.2" CACHE PATH "miniz")
set(PUGIXML_DIR "${THIRD_PARTY_DIR}/pugixml-1.13" CACHE PATH "pugixml")

add_library(miniz INTERFACE)
set_target_properties(miniz PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${MINIZ_DIR}")

add_library(pugixml INTERFACE)
set_target_properties(pugixml PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${PUGIXML_DIR}"
  INTERFACE_SOURCES             "${PUGIXML_DIR}/pugixml.cpp")

include(cmake/util.cmake)

if(BUILD_SHARED)
  add_library(minidocx SHARED)
  target_compile_definitions(minidocx
    PUBLIC  MINIDOCX_SHARED
    PRIVATE MINIDOCX_EXPORTS)
else()
  add_library(minidocx STATIC)
endif()
aux_inc_directory(include headers)
aux_src_directory(src sources)
source_group(TREE "${PROJECT_SOURCE_DIR}/include/minidocx"
  PREFIX "Header Files" FILES ${headers})
source_group(TREE "${PROJECT_SOURCE_DIR}/src"
  PREFIX "Source Files" FILES ${sources})
target_sources(minidocx PRIVATE ${headers} ${sources})
target_include_directories(minidocx PRIVATE include/minidocx)
target_include_directories(minidocx INTERFACE include)
target_compile_definitions(minidocx PRIVATE MINIDOCX_NAMESPACE=md)
target_link_libraries(minidocx PRIVATE miniz pugixml)
if(WITH_STATIC_CRT)
  set_target_properties(minidocx PROPERTIES
    MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT myapp)
endif()


if(BUILD_TESTS)
  enable_testing()
  add_executable(minidocx_regression tests/minidocx_regression.cpp)
  target_link_libraries(minidocx_regression PRIVATE minidocx)
  target_include_directories(minidocx_regression PRIVATE include include/minidocx)
  target_compile_definitions(minidocx_regression PRIVATE
    MINIDOCX_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    MINIDOCX_TESTDATA_DIR="${PROJECT_SOURCE_DIR}/testdata"
    MINIDOCX_TEST_OUTPUT_DIR="${PROJECT_BINARY_DIR}")
  add_test(NAME minidocx.regression COMMAND minidocx_regression)
  set_target_properties(minidocx_regression PROPERTIES FOLDER tests)

  add_executable(minidocx_inspection_pipeline tests/inspection_semantic_tests.cpp)
  target_link_libraries(minidocx_inspection_pipeline PRIVATE minidocx)
  target_include_directories(minidocx_inspection_pipeline PRIVATE include include/minidocx)
  add_test(NAME minidocx.inspection_pipeline COMMAND minidocx_inspection_pipeline)
  set_target_properties(minidocx_inspection_pipeline PROPERTIES FOLDER tests)

  add_executable(minidocx_commands tests/minidocx_commands_tests.cpp)
  target_link_libraries(minidocx_commands PRIVATE minidocx)
  target_include_directories(minidocx_commands PRIVATE include include/minidocx)
  add_test(NAME minidocx.commands COMMAND minidocx_commands)
  set_target_properties(minidocx_commands PROPERTIES FOLDER tests)

  add_custom_target(minidocx_validate
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "minidocx\."
    DEPENDS minidocx_regression minidocx_inspection_pipeline minidocx_commands
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMENT "Running minidocx branch validation suite")

  add_custom_target(minidocx_preintegration_gate
    DEPENDS minidocx_validate
    COMMENT "Running minidocx pre-integration readiness gate")
endif()
