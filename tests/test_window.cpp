#include <exd/app/window.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

namespace exd::app {
namespace {

TEST_CASE("Window: construction with default desc", "[window][display]") {
    Window win;
    if (!win.is_valid()) {
        SKIP("Window / GL context creation failed — no usable display");
    }
    CHECK(win.is_valid());
    CHECK_FALSE(win.should_close());
    CHECK(win.native_window() != nullptr);
    CHECK(win.native_context() != nullptr);
}

TEST_CASE("Window: custom title and size", "[window][display]") {
    WindowDesc desc;
    desc.title = "Test Window";
    desc.width = 800;
    desc.height = 600;

    Window win(desc);
    if (!win.is_valid()) {
        SKIP("Window creation failed");
    }

    int w = 0, h = 0; float aspect = 0;
    win.get_dimensions(w, h, aspect);
    CHECK(w == 800);
    CHECK(h == 600);
    CHECK(aspect == Catch::Approx(800.0f / 600.0f));
}

TEST_CASE("Window: set_title works", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK_NOTHROW(win.set_title("New Title"));
}

TEST_CASE("Window: set_input_mode and cursor_mode", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK(win.input_mode == InputMode::FPS);
    CHECK(win.cursor_mode() == CursorMode::Captured);

    win.set_input_mode(InputMode::UI);
    CHECK(win.input_mode == InputMode::UI);
    CHECK(win.cursor_mode() == CursorMode::Normal);

    win.set_cursor_mode(CursorMode::Hidden);
    CHECK(win.cursor_mode() == CursorMode::Hidden);
}

TEST_CASE("Window: was_key_released defaults to false", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK_FALSE(win.was_key_pressed(SDL_SCANCODE_A));
    CHECK_FALSE(win.was_key_pressed(SDL_SCANCODE_SPACE));
    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_A));
    CHECK_FALSE(win.was_key_released(SDL_SCANCODE_SPACE));
}

TEST_CASE("Window: mouse_button_down defaults to false", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK_FALSE(win.mouse_button_down(SDL_BUTTON_LEFT));
    CHECK_FALSE(win.mouse_button_down(SDL_BUTTON_RIGHT));
}

TEST_CASE("Window: poll_events does not crash", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK_NOTHROW(win.poll_events());
    CHECK_FALSE(win.should_close());
}

TEST_CASE("Window: swap_buffers does not crash", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK_NOTHROW(win.swap_buffers());
}

TEST_CASE("Window: reset_mouse_delta zeroes out", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    win.poll_events();
    win.reset_mouse_delta();
    CHECK(win.mouse_rel_x == 0.0f);
    CHECK(win.mouse_rel_y == 0.0f);
    CHECK(win.events().mouse_rel_x == 0.0f);
    CHECK(win.events().mouse_rel_y == 0.0f);
}

TEST_CASE("Window: close flag", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK_FALSE(win.should_close());
    win.close();
    CHECK(win.should_close());
}

TEST_CASE("Window: keyboard_state set after poll", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    win.poll_events();
    CHECK(win.keyboard_state != nullptr);
}

TEST_CASE("Window: events() accessor returns event state", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    const auto& es = win.events();
    CHECK(es.num_events >= 0);
}

TEST_CASE("Window: WindowDesc with MSAA", "[window][display]") {
    WindowDesc desc;
    desc.msaa_samples = 4;
    Window win(desc);
    if (!win.is_valid()) { SKIP("MSAA window creation failed"); }
    CHECK(win.is_valid());
}

TEST_CASE("Window: WindowDesc with vsync off", "[window][display]") {
    WindowDesc desc;
    desc.vsync = false;
    Window win(desc);
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK(win.is_valid());
}

TEST_CASE("Window: grid_visible and wireframe defaults", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK(win.grid_visible == true);
    CHECK(win.wireframe == false);
}

TEST_CASE("Window: scroll defaults to zero", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK(win.scroll_x == 0.0f);
    CHECK(win.scroll_y == 0.0f);
    CHECK(win.events().scroll_x == 0.0f);
    CHECK(win.events().scroll_y == 0.0f);
}

TEST_CASE("Window: set_icon does not crash with null", "[window][display]") {
    Window win;
    if (!win.is_valid()) { SKIP("Window creation failed"); }
    CHECK_NOTHROW(win.set_icon(nullptr));
}

} // namespace
} // namespace exd::app
