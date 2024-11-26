cmake_minimum_required(VERSION 3.5)

# Project setup
project(ChessVoid LANGUAGES CXX)

# Set C++ standard (C++20 in this case for compatibility)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

# Error if building out of a build directory
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with "
            "CMakeLists.txt file). Please make a build subdirectory. Feel free to "
            "remove CMakeCache.txt and CMakeFiles.")
endif ()

# Include FetchContent and ExternalProject
include(FetchContent)
include(ExternalProject)

enable_testing()

# Try to find raylib installed on the system
find_package(ftxui 5.0 QUIET)

# If raylib is not found, fetch it
if (NOT ftxui_FOUND)
    FetchContent_Declare(
            ftxui
            GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui.git
            GIT_TAG v5.0.0  # Use a stable version
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(ftxui)

    message(STATUS "Using built ftxui")
else ()
    message(STATUS "Using system ftxui")
endif ()

find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v3.4.0 # or a later release
    )

    FetchContent_MakeAvailable(Catch2)
    message(STATUS "Using built Catch2")
else()
    message(STATUS "Using system Catch2")
endif ()

FetchContent_Declare(
        libchess
        GIT_REPOSITORY https://github.com/kz04px/libchess.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(libchess)

# Collect all source files recursively
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE TST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/test_*.cpp")

list(FILTER SRC_FILES EXCLUDE REGEX ".*/main\\.cpp$")
list(FILTER SRC_FILES EXCLUDE REGEX ".*/test_.*\\.cpp$")

# Create the executable for your console application (main app)
add_executable(chess_void src/main.cpp ${SRC_FILES})

# Link ftxui library
target_include_directories(chess_void PRIVATE "${ftxui_SOURCE_DIR}/include" "${libchess_SOURCE_DIR}/src")
target_link_libraries(
        chess_void
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
        PRIVATE libchess_static
)

## Create the test executable
#add_executable(tests ${SRC_FILES} ${TST_FILES})
#target_include_directories(tests PRIVATE "${ftxui_SOURCE_DIR}/include")
#target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)