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

TEST_CASE("EventState: was_key_released returns false when key not released", "[event_state]") {
    EventState es;
    // Verify that set_state doesn't clobber the key_up_ array
    // (key_up_ is managed separately by reset_keys/set_key_up)
    SDL_Event ev = {};
    es.set_state(&ev, 0, nullptr, 0.0f, 0.0f);

    // was_key_released should still return false (key_up_ defaults to false)
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_SPACE));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_RETURN));
}

TEST_CASE("EventState: set_key_up makes was_key_released return true", "[event_state]") {
    EventState es;

    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_A));
    es.set_key_up(SDL_SCANCODE_A);
    CHECK(es.was_key_released(SDL_SCANCODE_A));

    // Other keys should remain unaffected
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_B));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_SPACE));
}

TEST_CASE("EventState: reset_keys clears all key-up states", "[event_state]") {
    EventState es;

    es.set_key_up(SDL_SCANCODE_A);
    es.set_key_up(SDL_SCANCODE_ESCAPE);
    es.set_key_up(SDL_SCANCODE_SPACE);

    CHECK(es.was_key_released(SDL_SCANCODE_A));
    CHECK(es.was_key_released(SDL_SCANCODE_ESCAPE));
    CHECK(es.was_key_released(SDL_SCANCODE_SPACE));

    es.reset_keys();

    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_A));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_ESCAPE));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_SPACE));
}

TEST_CASE("EventState: set_key_up and reset_keys simulate per-frame pattern", "[event_state]") {
    EventState es;

    // Frame 1: A released
    es.reset_keys();
    es.set_key_up(SDL_SCANCODE_A);
    CHECK(es.was_key_released(SDL_SCANCODE_A));

    // Frame 2: A is no longer "just released" — B is released instead
    es.reset_keys();
    es.set_key_up(SDL_SCANCODE_B);
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_A));
    CHECK(es.was_key_released(SDL_SCANCODE_B));

    // Frame 3: nothing released
    es.reset_keys();
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_A));
    CHECK_FALSE(es.was_key_released(SDL_SCANCODE_B));
}

TEST_CASE("EventState: set_key_up with out-of-range scancode is safe", "[event_state]") {
    EventState es;
    // Should not crash or corrupt memory
    CHECK_NOTHROW(es.set_key_up(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT)));
    CHECK_NOTHROW(es.set_key_up(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT + 1000)));
}

TEST_CASE("EventState: set_key_up does not affect fields set by set_state", "[event_state]") {
    EventState es;

    SDL_Event ev = {};
    es.set_state(&ev, 3, nullptr, 1.5f, -0.5f);
    es.set_key_up(SDL_SCANCODE_X);

    // set_state fields still intact
    CHECK(es.num_events == 3);
    CHECK(es.events == &ev);
    CHECK(es.mouse_rel_x == 1.5f);
    CHECK(es.mouse_rel_y == -0.5f);

    // key state independent
    CHECK(es.was_key_released(SDL_SCANCODE_X));
}

TEST_CASE("EventState: negative num_events is accepted", "[event_state]") {
    EventState es;
    // set_state accepts negative counts (caller's responsibility to validate)
    CHECK_NOTHROW(es.set_state(nullptr, -1, nullptr, 0.0f, 0.0f));
    CHECK(es.num_events == -1);
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
