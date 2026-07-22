#pragma once

#include <functional>
#include <string>

namespace exd::app::services {

/// Open a native file-open dialog.
///
/// @param title    Dialog title / prompt text.
/// @param filter   File-type filter (e.g. "*.png;*.jpg").
/// @param on_select Callback invoked with the selected path,
///                  or an empty string if the user cancelled.
///
/// @note Currently a stub; a native dialog implementation is pending.
void open_file_dialog(const std::string& title, const std::string& filter,
                      std::function<void(const std::string&)> on_select);

} // namespace exd::app::services
