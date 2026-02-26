cmake_minimum_required(VERSION 3.10)
project(pdf-outline LANGUAGES CXX)

# Set default target to pdf-outline (matches Makefile 'all' target)
set(CMAKE_DEFAULT_TARGET pdf-outline)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

# Cross-platform library dependencies:
# - ICU (libicu-uc, libicu-io): Required for Unicode support
# - nlohmann_json: Header-only JSON library
# - pindf: PDF manipulation library (included as subdirectory)
#
# On macOS with Homebrew: Add brew installation paths automatically
# On Linux: Install via package manager (e.g., libicu-dev, nlohmann-json-dev)
# On Windows: Use vcpkg or set CMAKE_PREFIX_PATH to library install locations

# On macOS, add Homebrew installation prefixes to CMAKE_PREFIX_PATH
# This helps find_package locate libraries installed via Homebrew
if(APPLE)
    # Standard Homebrew prefixes for Apple Silicon and Intel
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
    message(STATUS "Added Homebrew prefixes to CMAKE_PREFIX_PATH for macOS")

    # Add common library-specific opt paths for ICU and nlohmann_json
    set(BREW_OPT_PATHS
        "/opt/homebrew/opt/icu4c"
        "/opt/homebrew/opt/nlohmann-json"
        "/usr/local/opt/icu4c"
        "/usr/local/opt/nlohmann-json"
    )
    foreach(brew_path ${BREW_OPT_PATHS})
        if(EXISTS "${brew_path}")
            message(STATUS "Adding Homebrew opt path to CMAKE_PREFIX_PATH: ${brew_path}")
            list(APPEND CMAKE_PREFIX_PATH ${brew_path})
        endif()
    endforeach()
endif()

# Prefer Config mode over Module mode for find_package (better for Homebrew packages)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Find required libraries using CMake's find_package
find_package(ICU COMPONENTS uc io REQUIRED)
find_package(nlohmann_json 3.2 REQUIRED)

# Include pindf library
add_subdirectory(pindf)

# Source files for main library
set(MAIN_SRCS
    destination.cpp
    name_tree.cpp
    outline.cpp
    page_tree.cpp
    utils.cpp
)

# Create main executable
add_executable(pdf-outline main.cpp ${MAIN_SRCS})

# Include directories for main executable (only our own headers)
target_include_directories(pdf-outline PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pindf
)

# Link libraries (modern CMake uses imported targets)
target_link_libraries(pdf-outline PRIVATE pindf ICU::uc ICU::io nlohmann_json::nlohmann_json)

# Add compiler flags
target_compile_options(pdf-outline PRIVATE -Wall $<$<CONFIG:Debug>:-g>)

# Create static-linked executable
add_executable(pdf-outline-static main.cpp ${MAIN_SRCS})
target_include_directories(pdf-outline-static PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pindf
)
target_link_libraries(pdf-outline-static PRIVATE pindf_static ICU::uc ICU::io nlohmann_json::nlohmann_json)
target_compile_options(pdf-outline-static PRIVATE -Wall $<$<CONFIG:Debug>:-g>)

# Create test_extract executable
add_executable(test_extract test_extract.cpp ${MAIN_SRCS})
target_include_directories(test_extract PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pindf
)
target_link_libraries(test_extract PRIVATE pindf ICU::uc ICU::io nlohmann_json::nlohmann_json)
target_compile_options(test_extract PRIVATE -Wall $<$<CONFIG:Debug>:-g>)

# Create test_modif executable
add_executable(test_modif test_modif.cpp ${MAIN_SRCS})
target_include_directories(test_modif PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pindf
)
target_link_libraries(test_modif PRIVATE pindf ICU::uc ICU::io nlohmann_json::nlohmann_json)
target_compile_options(test_modif PRIVATE -Wall $<$<CONFIG:Debug>:-g>)

# Optional address sanitizer (matching pindf CMakeLists.txt)
option(PDF_OUTLINE_USE_ASAN "Enable address sanitizer" OFF)
if(PDF_OUTLINE_USE_ASAN)
    target_compile_options(pdf-outline PRIVATE -fsanitize=address)
    target_link_options(pdf-outline PRIVATE -fsanitize=address)
    target_compile_options(pdf-outline-static PRIVATE -fsanitize=address)
    target_link_options(pdf-outline-static PRIVATE -fsanitize=address)
    target_compile_options(test_extract PRIVATE -fsanitize=address)
    target_link_options(test_extract PRIVATE -fsanitize=address)
    target_compile_options(test_modif PRIVATE -fsanitize=address)
    target_link_options(test_modif PRIVATE -fsanitize=address)
endif()

# Custom target 'tests' to build test executables (matching Makefile's test target)
add_custom_target(tests DEPENDS test_extract test_modif)

# Custom target 'static' to build static-linked executable
add_custom_target(static DEPENDS pdf-outline-static)

# Custom target 'clean' that invokes cmake --build --target clean
add_custom_target(clean_cmake
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "Cleaning build directory"
)

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")