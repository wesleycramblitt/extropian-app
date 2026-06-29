// Reusable Console component — C++ side.
// Append log entries with severity coloring.

#include <exd/app/ui_host.hpp>
#include <string>
#include <vector>

namespace exd::app::rmlui {

enum class LogSeverity { Info, Warning, Error, Success, Debug };

struct LogEntry {
    LogSeverity severity;
    std::string message;
};

inline void append_log(Document& doc, const std::string& container_id,
                        LogSeverity severity, const std::string& message) {
    const char* type_str = "info";
    switch (severity) {
        case LogSeverity::Error:   type_str = "error";   break;
        case LogSeverity::Warning: type_str = "warning";  break;
        case LogSeverity::Success: type_str = "success";  break;
        case LogSeverity::Debug:   type_str = "debug";    break;
        default:                   type_str = "info";     break;
    }

    std::string entry = "<div class=\"log-entry\" type=\"" + std::string(type_str) + "\">";
    entry += message;
    entry += "</div>";

    // Append to console — we use set_inner_rml with the old content + new entry
    // In practice, you'd maintain the full log buffer in C++ and rebuild.
    doc.set_inner_rml(container_id,
        "<div class=\"log-entry\" type=\"" + std::string(type_str) + "\">"
        + message + "</div>");
}

} // namespace exd::app::rmlui
