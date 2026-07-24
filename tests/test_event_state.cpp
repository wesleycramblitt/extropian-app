#include <exd/app/event_state.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

namespace exd::app {
namespace {

TEST_CASE("EventState: default construction", "[event_state]") {
    EventState es;
    CHECK(es.num_events == 0);
    CHECK(es.events == nullptr);
    CHECK(es.keyboard_state == nullptr);
    CHECK(es.mouse_rel_x == 0.0f);
    CHECK(es.mouse_rel_y == 0.0f);
}

TEST_CASE("EventState: was_key_pressed returns false by default", "[event_state]") {
    EventState es;
    for (int i = 0; i < 100; ++i) {
        CHECK_FALSE(es.was_key_pressed(static_cast<SDL_Scancode>(i)));
    }
}

TEST_CASE("EventState: was_key_released returns false by default", "[event_state]") {
    EventState es;
    for (int i = 0; i < 100; ++i) {
        CHECK_FALSE(es.was_key_released(static_cast<SDL_Scancode>(i)));
    }
}

TEST_CASE("EventState: set_key_down makes was_key_pressed return true", "[event_state]") {
    EventState es;
    CHECK_FALSE(es.was_key_pressed(SDL_SCANCODE_SPACE));
    es.set_key_down(SDL_SCANCODE_SPACE);
    CHECK(es.was_key_pressed(SDL_SCANCODE_SPACE));
    CHECK_FALSE(es.was_key_pressed(SDL_SCANCODE_A));
}

TEST_CASE("EventState: set_key_up makes was_key_released return true", "[event_state]") {
    EventState es;
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_A));
    es.set_key_up(SDL_SCANCODE_A);
    CHECK(es.was_key_released(SDL_SCANCODE_A));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_B));
}

TEST_CASE("EventState: begin_frame clears all key-up states", "[event_state]") {
    EventState es;
    es.set_key_up(SDL_SCANCODE_A);
    es.set_key_up(SDL_SCANCODE_ESCAPE);
    CHECK(es.was_key_released(SDL_SCANCODE_A));
    es.begin_frame();
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_A));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_ESCAPE));
}

TEST_CASE("EventState: begin_frame resets per-frame state", "[event_state]") {
    EventState es;
    es.set_key_down(SDL_SCANCODE_X);
    es.set_key_up(SDL_SCANCODE_Y);
    es.set_mouse_button(SDL_BUTTON_LEFT, true);
    es.scroll_x = 5.0f;
    es.scroll_y = 3.0f;
    es.mouse_rel_x = 100.0f; // public field, should survive

    es.begin_frame();

    CHECK_FALSE(es.was_key_pressed(SDL_SCANCODE_X));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_Y));
    CHECK_FALSE(es.mouse_button_down(SDL_BUTTON_LEFT));
    CHECK(es.scroll_x == 0.0f);
    CHECK(es.scroll_y == 0.0f);
    CHECK(es.mouse_rel_x == 100.0f); // accumulated delta survives
}

TEST_CASE("EventState: set_key_up out-of-range is safe", "[event_state]") {
    EventState es;
    CHECK_NOTHROW(es.set_key_up(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT + 1000)));
}

TEST_CASE("EventState: set_key_down out-of-range is safe", "[event_state]") {
    EventState es;
    CHECK_NOTHROW(es.set_key_down(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT + 1000)));
}

TEST_CASE("EventState: mouse_button_down defaults to false", "[event_state]") {
    EventState es;
    CHECK_FALSE(es.mouse_button_down(SDL_BUTTON_LEFT));
    CHECK_FALSE(es.mouse_button_down(SDL_BUTTON_RIGHT));
    CHECK_FALSE(es.mouse_button_down(SDL_BUTTON_MIDDLE));
    CHECK_FALSE(es.mouse_button_down(42));
}

TEST_CASE("EventState: set_mouse_button roundtrip", "[event_state]") {
    EventState es;
    CHECK_FALSE(es.mouse_button_down(SDL_BUTTON_LEFT));
    es.set_mouse_button(SDL_BUTTON_LEFT, true);
    CHECK(es.mouse_button_down(SDL_BUTTON_LEFT));
    es.set_mouse_button(SDL_BUTTON_LEFT, false);
    CHECK_FALSE(es.mouse_button_down(SDL_BUTTON_LEFT));
}

TEST_CASE("EventState: set_mouse_button multiple buttons", "[event_state]") {
    EventState es;
    es.set_mouse_button(SDL_BUTTON_LEFT, true);
    es.set_mouse_button(SDL_BUTTON_RIGHT, true);
    CHECK(es.mouse_button_down(SDL_BUTTON_LEFT));
    CHECK(es.mouse_button_down(SDL_BUTTON_RIGHT));
    CHECK_FALSE(es.mouse_button_down(SDL_BUTTON_MIDDLE));
}

TEST_CASE("EventState: set_mouse_button out-of-range is safe", "[event_state]") {
    EventState es;
    CHECK_NOTHROW(es.set_mouse_button(999, true));
    CHECK_NOTHROW(es.set_mouse_button(-1, false));
}

TEST_CASE("EventState: scroll defaults to zero", "[event_state]") {
    EventState es;
    CHECK(es.scroll_x == 0.0f);
    CHECK(es.scroll_y == 0.0f);
}

TEST_CASE("EventState: gamepad defaults", "[event_state]") {
    EventState es;
    CHECK(es.gamepad_left_x == 0.0f);
    CHECK(es.gamepad_left_y == 0.0f);
    CHECK(es.gamepad_right_x == 0.0f);
    CHECK(es.gamepad_right_y == 0.0f);
    CHECK(es.gamepad_left_trigger == 0.0f);
    CHECK(es.gamepad_right_trigger == 0.0f);
    CHECK(es.gamepad_buttons == 0);
}

} // namespace
} // namespace exd::app
