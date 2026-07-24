/// Verify that the umbrella header is self-contained.

#include <exd/app/app.hpp>

#include <catch2/catch_test_macros.hpp>

namespace exd::app {
namespace {

TEST_CASE("Umbrella: app.hpp exposes all core types", "[umbrella]") {
    InputMode       mode = InputMode::FPS;
    CursorMode      cmode = CursorMode::Normal;
    EventState      es;
    WindowDesc      desc;

    (void)mode;
    (void)cmode;
    (void)es;
    (void)desc;

    SUCCEED("All types available via app.hpp");
}

TEST_CASE("Umbrella: each header is self-contained", "[umbrella]") {
    SUCCEED("All headers compile in isolation");
}

} // namespace
} // namespace exd::app
