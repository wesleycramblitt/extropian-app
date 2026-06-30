cmake_minimum_required(VERSION 3.21)
project(extropian-app VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "Extropian App: application framework with RmlUi integration, modes, system graph"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies: use FetchContent when not already available ──
include(FetchContent)

if(NOT TARGET exd::core)
    FetchContent_Declare(exd-core
        GIT_REPOSITORY https://github.com/wesleycramblitt/extropian-core.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(exd-core)
endif()

if(NOT TARGET exd::render)
    FetchContent_Declare(exd-render
        GIT_REPOSITORY https://github.com/wesleycramblitt/extropian-render.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(exd-render)
endif()
find_package(OpenGL REQUIRED)

# RmlUi — disabled (needs API compatibility work)
# include(FetchContent)
# FetchContent_Declare(RmlUi ...)
# FetchContent_MakeAvailable(RmlUi)
# find_package(Freetype REQUIRED)

# ── Library target ────────────────────────────────────
add_library(exd-app STATIC
    # Core application framework
    src/application.cpp
    src/mode.cpp
    src/system_graph.cpp
    src/command.cpp
    src/project.cpp
    src/asset_watcher.cpp
    src/input_router.cpp
    src/layout.cpp
    src/profile.cpp

    # Window / event system
    src/window.cpp
    src/event_state.cpp
    src/camera_controller_system.cpp

    # Services
    src/services/log_service.cpp
    src/services/clipboard_service.cpp
    src/services/file_dialog_service.cpp
    src/services/notification_service.cpp

    # RmlUi integration — disabled for now
    # src/rmlui/host.cpp
    # src/rmlui/render_interface.cpp
    # src/rmlui/system_interface.cpp
    # src/rmlui/data_binding.cpp
    # src/rmlui/ui_host_factory.cpp
    # src/rmlui/components/*.cpp
)

target_include_directories(exd-app PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_include_directories(exd-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(exd-app PUBLIC
    exd::core
    exd::render
    OpenGL::GL
)

add_library(exd::app ALIAS exd-app)

# Copy RmlUi assets (RML + RCSS + fonts) to build directory
add_custom_command(TARGET exd-app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:exd-app>/assets
)

# ── Install ───────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS exd-app EXPORT exd-app-targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/exd-app/assets)
install(EXPORT exd-app-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/exd-app)

# ── Tests ─────────────────────────────────────────────
option(EXD_APP_BUILD_TESTS "Build tests" OFF)
if(EXD_APP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
