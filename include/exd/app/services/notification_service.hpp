#pragma once

#include <string>

namespace exd::app::services {

/// Queue a notification for display.
///
/// @param title      Notification title.
/// @param message    Notification body text.
/// @param duration_ms Display duration in milliseconds (default 3000).
void notify(const std::string& title, const std::string& message, int duration_ms = 3000);

} // namespace exd::app::services
