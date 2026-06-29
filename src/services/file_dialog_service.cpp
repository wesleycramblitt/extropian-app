#include <string>
#include <functional>
#include <cstdio>
namespace exd::app::services {
void open_file_dialog(const std::string& title, const std::string& filter,
                      std::function<void(const std::string&)> on_select) {
    // TODO: Use native file dialog (SDL3 file dialog or platform API)
    std::fprintf(stderr, "[file_dialog] %s (%s) — native dialog not yet implemented\n",
                 title.c_str(), filter.c_str());
    on_select("");
}
}
