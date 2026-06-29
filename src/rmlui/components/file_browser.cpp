// FileBrowser component — C++ stub.
// In the future: native file dialog or RmlUi-based file browser.

#include <string>
#include <functional>

namespace exd::app::rmlui {

inline void open_file_browser(const std::string& title,
                               const std::string& filter,
                               std::function<void(const std::string& path)> on_select) {
    // TODO: Use native file dialog (SDL3 or platform) or build RmlUi browser.
    (void)title;
    (void)filter;
    (void)on_select;
}

} // namespace exd::app::rmlui
