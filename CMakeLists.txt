cmake_minimum_required(VERSION 3.1)
project(saucer-cli LANGUAGES CXX VERSION 1.1)

# --------------------------------------------------------------------------------------------------------
# Create library
# --------------------------------------------------------------------------------------------------------

file(GLOB src "src/*.cpp")

add_executable(${PROJECT_NAME} ${src})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "saucer")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)

if (UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -Wextra -pedantic)
endif()

# --------------------------------------------------------------------------------------------------------
# Include "include" folder
# --------------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PUBLIC "include" "templates")

# --------------------------------------------------------------------------------------------------------
# CMake Header
# --------------------------------------------------------------------------------------------------------

set(VERSION ${PROJECT_VERSION})
set(ARCH ${CMAKE_SYSTEM_PROCESSOR})
set(COMPILER ${CMAKE_CXX_COMPILER_ID})

configure_file(info.hpp.in info.hpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})