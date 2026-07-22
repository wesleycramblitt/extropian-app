#include <exd/app/event_state.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <array>

namespace exd::app {
namespace {

TEST_CASE("EventState: default construction", "[event_state]") {
    EventState es;
    // All key-up entries should be false
    CHECK(es.num_events == 0);
    CHECK(es.events == nullptr);
    CHECK(es.keyboard_state == nullptr);
    CHECK(es.mouse_rel_x == 0.0f);
    CHECK(es.mouse_rel_y == 0.0f);
}

TEST_CASE("EventState: was_key_released returns false for all keys by default", "[event_state]") {
    EventState es;
    for (int i = 0; i < SDL_SCANCODE_COUNT; ++i) {
        INFO("Scancode: " << i);
        CHECK_FALSE(es.was_key_released(static_cast<SDL_Scancode>(i)));
    }
}

TEST_CASE("EventState: parameterized construction sets all fields", "[event_state]") {
    SDL_Event ev = {};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.scancode = SDL_SCANCODE_A;

    const bool* kb = nullptr; // Don't need real pointer for this test
    EventState es(&ev, 1, kb, 3.5f, -2.0f);

    CHECK(es.num_events == 1);
    CHECK(es.events == &ev);
    CHECK(es.keyboard_state == kb);
    CHECK(es.mouse_rel_x == 3.5f);
    CHECK(es.mouse_rel_y == -2.0f);
}

TEST_CASE("EventState: set_state updates all fields", "[event_state]") {
    EventState es;

    SDL_Event ev = {};
    ev.type = SDL_EVENT_MOUSE_MOTION;

    es.set_state(&ev, 5, nullptr, 10.0f, 20.0f);

    CHECK(es.num_events == 5);
    CHECK(es.events == &ev);
    CHECK(es.keyboard_state == nullptr);
    CHECK(es.mouse_rel_x == 10.0f);
    CHECK(es.mouse_rel_y == 20.0f);
}

TEST_CASE("EventState: was_key_released respects key_up_ array (via Window friend)", "[event_state]") {
    // We can't directly set key_up_ (private, Window is friend),
    // but we can verify the default state and test through the
    // public interface that the array is internally consistent.

    EventState es;
    // Verify that set_state doesn't clobber the key_up_ array
    // (key_up_ is managed separately by Window::poll_events)
    SDL_Event ev = {};
    es.set_state(&ev, 0, nullptr, 0.0f, 0.0f);

    // was_key_released should still return false (key_up_ defaults to false)
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_SPACE));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_RETURN));
}

TEST_CASE("EventState: scancode out of bounds is handled", "[event_state]") {
    EventState es;
    // SDL_SCANCODE_COUNT is 295+ in SDL3; this should not crash
    // with the array bounds check in was_key_released
    CHECK_FALSE(es.was_key_released(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT)));
    CHECK_FALSE(es.was_key_released(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT + 100)));
}

TEST_CASE("EventState: multiple set_state calls track latest values", "[event_state]") {
    EventState es;

    SDL_Event ev1 = {};
    es.set_state(&ev1, 1, nullptr, 1.0f, 2.0f);
    CHECK(es.mouse_rel_x == 1.0f);
    CHECK(es.mouse_rel_y == 2.0f);
    CHECK(es.num_events == 1);

    SDL_Event ev2 = {};
    es.set_state(&ev2, 3, nullptr, 42.0f, 0.0f);
    CHECK(es.mouse_rel_x == 42.0f);
    CHECK(es.mouse_rel_y == 0.0f);
    CHECK(es.num_events == 3);
    CHECK(es.events == &ev2);
}

TEST_CASE("EventState: zero num_events is valid", "[event_state]") {
    EventState es;
    es.set_state(nullptr, 0, nullptr, 0.0f, 0.0f);
    CHECK(es.num_events == 0);
}

} // namespace
} // namespace exd::app
