#pragma once

#include <string>

namespace exd::app::services {

/// Set the system clipboard text.
void clipboard_set(const std::string& text);

/// Get the current system clipboard text.
/// Returns an empty string if the clipboard is empty or unavailable.
[[nodiscard]] std::string clipboard_get();

} // namespace exd::app::services
