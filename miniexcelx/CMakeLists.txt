add_library(miniexcelx
    src/Cell.cpp
    src/Workbook.cpp
    src/Worksheet.cpp
)

target_include_directories(miniexcelx
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(miniexcelx PUBLIC cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(miniexcelx PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(miniexcelx PRIVATE /W4)
endif()

add_executable(miniexcelx_smoke_test
    tests/SmokeTest.cpp
)

target_link_libraries(miniexcelx_smoke_test PRIVATE miniexcelx)

add_test(NAME miniexcelx_smoke_test COMMAND miniexcelx_smoke_test)
