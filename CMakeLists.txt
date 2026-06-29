cmake_minimum_required(VERSION 3.21)
project(extropian-app VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "Extropian App: application framework with RmlUi integration, modes, system graph"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ──────────────────────────────────────
find_package(ext-core REQUIRED)
find_package(ext-render REQUIRED)

# RmlUi
find_package(RmlUi CONFIG QUIET)
if(NOT RmlUi_FOUND)
    include(FetchContent)
    FetchContent_Declare(RmlUi
        GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
        GIT_TAG master
    )
    set(RMLUI_BACKEND MATRIX_GL2 CACHE STRING "" FORCE)
    FetchContent_MakeAvailable(RmlUi)
endif()

# Freetype (for RmlUi text rendering)
find_package(Freetype REQUIRED)

# ── Library target ────────────────────────────────────
add_library(ext-app STATIC
    # Core
    src/application.cpp
    src/mode.cpp
    src/system_graph.cpp
    src/command.cpp
    src/project.cpp
    src/asset_watcher.cpp
    src/input_router.cpp
    src/layout.cpp
    src/profile.cpp

    # Services
    src/services/log_service.cpp
    src/services/clipboard_service.cpp
    src/services/file_dialog_service.cpp
    src/services/notification_service.cpp

    # RmlUi integration
    src/rmlui/host.cpp
    src/rmlui/render_interface.cpp
    src/rmlui/system_interface.cpp
    src/rmlui/data_binding.cpp

    # RmlUi reusable components
    src/rmlui/components/property_editor.cpp
    src/rmlui/components/tree_view.cpp
    src/rmlui/components/viewport_3d.cpp
    src/rmlui/components/console.cpp
    src/rmlui/components/curve_editor.cpp
    src/rmlui/components/color_picker.cpp
    src/rmlui/components/file_browser.cpp
)

target_include_directories(ext-app PUBLIC include)
target_include_directories(ext-app PRIVATE src)

target_link_libraries(ext-app PUBLIC
    ext::core
    ext::render
    RmlCore
    RmlControls
    Freetype::Freetype
)

add_library(ext::app ALIAS ext-app)

# Copy RmlUi assets to build directory
add_custom_command(TARGET ext-app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:ext-app>/assets
)

# ── Install ───────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS ext-app EXPORT ext-app-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/ext-app/assets)
install(EXPORT ext-app-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ext-app)

# ── Tests ─────────────────────────────────────────────
option(EXT_APP_BUILD_TESTS "Build tests" OFF)
if(EXT_APP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
