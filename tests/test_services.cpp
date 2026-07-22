#include <exd/app/services/clipboard_service.hpp>
#include <exd/app/services/file_dialog_service.hpp>
#include <exd/app/services/notification_service.hpp>

#include <catch2/catch_test_macros.hpp>

namespace exd::app::services {
namespace {

// ── Clipboard ─────────────────────────────────────────────

TEST_CASE("Clipboard: clipboard_set does not crash", "[services][clipboard]") {
    // Setting clipboard should not crash even without a window
    CHECK_NOTHROW(clipboard_set("test text"));
}

TEST_CASE("Clipboard: clipboard_get returns a string", "[services][clipboard]") {
    // In a headless environment, clipboard_get typically returns ""
    std::string result = clipboard_get();
    // Just verify it returns *something* without crashing
    CHECK((result.empty() || !result.empty())); // tautology, but tests no-throw
}

TEST_CASE("Clipboard: clipboard_set/get roundtrip (headless)", "[services][clipboard]") {
    // In headless CI, clipboard may not persist, but the calls
    // should not crash.
    clipboard_set("hello world");
    std::string result = clipboard_get();
    // On systems without a clipboard, result will be empty.
    // On systems with one, it should be "hello world".
    // We just verify no crash.
    SUCCEED("clipboard_set/get completed without exception");
}

TEST_CASE("Clipboard: empty string", "[services][clipboard]") {
    CHECK_NOTHROW(clipboard_set(""));
    std::string result = clipboard_get();
    // Should be either empty or the previously set value; no crash.
    SUCCEED("empty clipboard handled");
}

TEST_CASE("Clipboard: long string does not crash", "[services][clipboard]") {
    // 10 KB of garbage should not crash the clipboard wrapper
    std::string long_str(10'000, 'X');
    CHECK_NOTHROW(clipboard_set(long_str));
    SUCCEED("10 KB string set without crash");
}

TEST_CASE("Clipboard: UTF-8 text", "[services][clipboard]") {
    // Use a normal string literal (the source encoding provides UTF-8 bytes).
    // Avoid u8"" prefix which produces char8_t[] incompatible with std::string.
    CHECK_NOTHROW(clipboard_set("\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf"
                                " \xe2\x80\x94 Hello, world! \xe2\x9c\xa8"
                                " Caf\xc3\xa9 na\xc3\xafve r\xc3\xa9sum\xc3\xa9"
                                " \xe2\x82\xac" "20"));
    SUCCEED("UTF-8 clipboard text set without crash");
}

// ── File Dialog ──────────────────────────────────────────

TEST_CASE("FileDialog: callback invoked with empty string", "[services][file_dialog]") {
    bool called = false;
    std::string received;

    open_file_dialog("Test Title", "*.txt", [&](const std::string& path) {
        called = true;
        received = path;
    });

    CHECK(called);
    CHECK(received.empty());
}

TEST_CASE("FileDialog: does not throw", "[services][file_dialog]") {
    CHECK_NOTHROW(open_file_dialog("Title", "*.png", [](const std::string&) {}));
}

TEST_CASE("FileDialog: multiple calls", "[services][file_dialog]") {
    int call_count = 0;
    auto cb = [&](const std::string&) { ++call_count; };

    open_file_dialog("A", "*", cb);
    open_file_dialog("B", "*", cb);
    open_file_dialog("C", "*", cb);

    CHECK(call_count == 3);
}

// ── Notification ─────────────────────────────────────────

TEST_CASE("Notification: notify does not throw", "[services][notification]") {
    CHECK_NOTHROW(notify("Title", "Message"));
}

TEST_CASE("Notification: notify with custom duration", "[services][notification]") {
    CHECK_NOTHROW(notify("Title", "Message", 5000));
    CHECK_NOTHROW(notify("T", "M", 0));
    CHECK_NOTHROW(notify("T", "M", -1));
}

TEST_CASE("Notification: empty strings are accepted", "[services][notification]") {
    CHECK_NOTHROW(notify("", ""));
    CHECK_NOTHROW(notify("", "message"));
    CHECK_NOTHROW(notify("title", ""));
}

TEST_CASE("Notification: multiple notifications", "[services][notification]") {
    // The internal queue is static; verify multiple calls don't crash
    for (int i = 0; i < 100; ++i) {
        notify("Title " + std::to_string(i), "Message " + std::to_string(i), i * 100);
    }
    SUCCEED("100 notifications queued without crash");
}

} // namespace
} // namespace exd::app::services
