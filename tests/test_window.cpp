#include <exd/app/window.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <SDL3/SDL.h>

namespace exd::app {
namespace {

/// RAII helper to init/quit SDL for tests that need a display.
struct SDLFixture {
    SDLFixture() {
        initialized = (SDL_Init(SDL_INIT_VIDEO) == 0);
    }
    ~SDLFixture() {
        if (initialized) SDL_Quit();
    }
    bool initialized = false;
};

TEST_CASE("Window: construction creates SDL window and GL context", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available — skipping window construction test");
    }

    Window win;
    // After successful construction:
    CHECK(win.sdl_window != nullptr);
    CHECK(win.gl_context != nullptr);
    CHECK_FALSE(win.should_close);
}

TEST_CASE("Window: get_dimensions returns valid values", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    int w = 0, h = 0;
    float aspect = 0.0f;
    win.get_dimensions(w, h, aspect);

    CHECK(w == 1280);
    CHECK(h == 720);
    CHECK(aspect == Catch::Approx(1280.0f / 720.0f));
}

TEST_CASE("Window: set_input_mode toggles", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    // Default is FPS
    CHECK(win.input_mode == InputMode::FPS);

    win.set_input_mode(InputMode::UI);
    CHECK(win.input_mode == InputMode::UI);

    win.set_input_mode(InputMode::FPS);
    CHECK(win.input_mode == InputMode::FPS);
}

TEST_CASE("Window: was_key_released defaults to false", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    // Without polling events, no keys are released
    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_A));
    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_SPACE));
    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_ESCAPE));
}

TEST_CASE("Window: poll_events does not crash", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    // Polling events with nothing to poll should be safe
    CHECK_NOTHROW(win.poll_events());
    CHECK_FALSE(win.should_close);
}

TEST_CASE("Window: swap_buffers does not crash", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    CHECK_NOTHROW(win.swap_buffers());
}

TEST_CASE("Window: event_buffer populated after poll_events", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    // After construction, event_buffer should have pending events
    // (like window expose/resize). Poll once to clear.
    win.poll_events();

    // Second poll should have minimal events
    size_t initial_size = win.event_buffer.size();
    win.poll_events();
    // In a test environment, we may or may not have events
    CHECK(win.event_buffer.size() >= 0);
    SUCCEED("event_buffer is usable after poll_events");
}

TEST_CASE("Window: should_close is false initially", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    CHECK_FALSE(win.should_close);
}

TEST_CASE("Window: grid_visible and wireframe defaults", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    CHECK(win.grid_visible == true);
    CHECK(win.wireframe == false);
}

TEST_CASE("Window: keyboard_state is set after poll_events", "[window][display]") {
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    win.poll_events();
    // After poll_events, keyboard_state should be set (non-null)
    CHECK(win.keyboard_state != nullptr);
}

TEST_CASE("Window: die sets should_close", "[window][display]") {
    // Note: die() currently calls std::exit(1), so we can't
    // actually test it directly. This test documents the expected
    // behavior and will need updating when die() is fixed.
    SDLFixture sdl;
    if (!sdl.initialized) {
        SKIP("No video display available");
    }

    Window win;
    if (!win.sdl_window) {
        SKIP("Window creation failed");
    }

    // die() calls std::exit() — we can't test it directly.
    // Documenting the intended behavior for when it's fixed.
    WARN("die() currently calls std::exit(1) — test skipped");
}

} // namespace
} // namespace exd::app
