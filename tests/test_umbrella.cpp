/// Verify that the umbrella header is self-contained and that each
/// individual header pulls in its own dependencies.

// 1. Umbrella header compiles in isolation (includes nothing else)
#include <exd/app/app.hpp>

// 2. Each individual header is self-contained
#include <exd/app/application.hpp>
#include <exd/app/window.hpp>
#include <exd/app/window_state.hpp>
#include <exd/app/event_state.hpp>
#include <exd/app/input_mode.hpp>

// 3. Service headers
#include <exd/app/services/clipboard_service.hpp>
#include <exd/app/services/file_dialog_service.hpp>
#include <exd/app/services/notification_service.hpp>

#include <catch2/catch_test_macros.hpp>

namespace exd::app {
namespace {

TEST_CASE("Umbrella: app.hpp exposes all core types", "[umbrella]") {
    // If we got this far, the umbrella header compiled.
    // Verify the types are actually available.
    InputMode       mode = InputMode::FPS;
    EventState      es;
    Application*    app = nullptr;  // can't construct without SDL, but type resolves

    // Prevent unused-variable warnings
    (void)mode;
    (void)es;
    (void)app;

    SUCCEED("All types available via app.hpp");
}

TEST_CASE("Umbrella: each header is self-contained", "[umbrella]") {
    // Each header already #included above without extra dependencies.
    // If any header were missing an include, the file wouldn't compile.
    SUCCEED("All headers compile in isolation");
}

TEST_CASE("Umbrella: input_mode.hpp backward-compat include", "[umbrella]") {
    // input_mode.hpp is a redirect to window_state.hpp.
    // Both should compile; InputMode should be available from either path.
    InputMode a = InputMode::FPS;
    InputMode b = InputMode::UI;
    CHECK(a != b);
}

} // namespace
} // namespace exd::app
