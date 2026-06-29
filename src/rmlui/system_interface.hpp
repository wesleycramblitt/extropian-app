#pragma once

#include <RmlUi/Core/SystemInterface.h>
#include <chrono>

namespace exd::app::rmlui {

/// @brief System interface for RmlUi — provides time, logging, clipboard.
class SystemInterface_SDL : public Rml::SystemInterface {
public:
    SystemInterface_SDL();

    /// Returns elapsed time in seconds since this interface was created.
    double GetElapsedTime() override;

    /// Log a message through the application's logging system.
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

    /// Set clipboard text (for copy/paste in text inputs).
    void SetClipboardText(const Rml::String& text) override;

    /// Get clipboard text.
    void GetClipboardText(Rml::String& text) override;

private:
    std::chrono::steady_clock::time_point start_time_;
    std::string clipboard_;
};

} // namespace exd::app::rmlui
