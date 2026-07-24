cmake_minimum_required(VERSION 3.21)
project(extropian-app VERSION 0.1.0 LANGUAGES C CXX
    DESCRIPTION "Extropian App: minimal application framework (window + events + main loop)"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ──────────────────────────────────────
include(FetchContent)

if(NOT TARGET exd::core)
    FetchContent_Declare(exd-core
        GIT_REPOSITORY https://github.com/wesleycramblitt/extropian-core.git
        GIT_TAG main
    )
    # Core tests pull in doctest 2.4.11 which is incompatible with
    # modern CMake — disable them since we only need the library.
    set(EXT_CORE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(exd-core)
endif()

find_package(OpenGL REQUIRED)

find_package(SDL3 CONFIG QUIET)
if(NOT SDL3_FOUND)
    include(FetchContent)
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.18
    )
    FetchContent_MakeAvailable(SDL3)
endif()

# ── Library target ────────────────────────────────────
add_library(exd-app STATIC
    src/application.cpp
    src/window.cpp
    src/event_state.cpp
)

target_include_directories(exd-app PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_include_directories(exd-app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# GLAD — vendored OpenGL 4.6 Core loader
target_include_directories(exd-app PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/include>"
)
target_sources(exd-app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/src/gl.c
)

target_link_libraries(exd-app PUBLIC
    exd::core
    OpenGL::GL
    SDL3::SDL3
)

add_library(exd::app ALIAS exd-app)

# ── Tests ──────────────────────────────────────────────
option(EXD_APP_BUILD_TESTS "Build extropian-app tests" ON)

if(EXD_APP_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.0
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(exd-app-tests
        tests/test_event_state.cpp
        tests/test_window_state.cpp
        tests/test_application.cpp
        tests/test_umbrella.cpp
    )

    target_link_libraries(exd-app-tests PRIVATE
        exd::app
        Catch2::Catch2WithMain
    )

    target_include_directories(exd-app-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    # Separate executable for tests that need a display (SDL3)
    add_executable(exd-app-tests-window
        tests/test_window.cpp
    )

    target_link_libraries(exd-app-tests-window PRIVATE
        exd::app
        Catch2::Catch2WithMain
    )

    target_include_directories(exd-app-tests-window PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(exd-app-tests)
    catch_discover_tests(exd-app-tests-window)
endif()
