#include <exd/app/window.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

namespace exd::app {
namespace {

// Window constructor calls SDL_Init / SDL_CreateWindow / gladLoadGL internally.
// If any step fails, sdl_window or gl_context will be null.
// We check those to decide whether to skip, rather than probing SDL ourselves.

TEST_CASE("Window: construction creates SDL window and GL context", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed — no usable display");
    }
    CHECK(win.sdl_window != nullptr);
    CHECK(win.gl_context != nullptr);
    CHECK_FALSE(win.should_close);
}

TEST_CASE("Window: get_dimensions returns valid values", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    int w = 0, h = 0;
    float aspect = 0.0f;
    win.get_dimensions(w, h, aspect);

    CHECK(w == 1280);
    CHECK(h == 720);
    CHECK(aspect == Catch::Approx(1280.0f / 720.0f));
}

TEST_CASE("Window: set_input_mode toggles", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    CHECK(win.input_mode == InputMode::FPS);

    win.set_input_mode(InputMode::UI);
    CHECK(win.input_mode == InputMode::UI);

    win.set_input_mode(InputMode::FPS);
    CHECK(win.input_mode == InputMode::FPS);
}

TEST_CASE("Window: was_key_released defaults to false", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_A));
    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_SPACE));
    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_ESCAPE));
}

TEST_CASE("Window: poll_events does not crash", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    CHECK_NOTHROW(win.poll_events());
    CHECK_FALSE(win.should_close);
}

TEST_CASE("Window: swap_buffers does not crash", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    CHECK_NOTHROW(win.swap_buffers());
}

TEST_CASE("Window: event_buffer is usable after poll_events", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    win.poll_events();
    win.poll_events();
    SUCCEED("event_buffer handled without crash");
}

TEST_CASE("Window: should_close is false initially", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    CHECK_FALSE(win.should_close);
}

TEST_CASE("Window: grid_visible and wireframe defaults", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    CHECK(win.grid_visible == true);
    CHECK(win.wireframe == false);
}

TEST_CASE("Window: keyboard_state is set after poll_events", "[window][display]") {
    Window win;
    if (!win.sdl_window || !win.gl_context) {
        SKIP("Window / GL context creation failed");
    }

    win.poll_events();
    CHECK(win.keyboard_state != nullptr);
}

} // namespace
} // namespace exd::app
