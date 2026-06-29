#include "system_interface.hpp"
#include <cstdio>
#include <SDL3/SDL.h>
#include <chrono>

namespace exd::app::rmlui {

SystemInterface_SDL::SystemInterface_SDL()
    : start_time_(std::chrono::steady_clock::now()) {}

double SystemInterface_SDL::GetElapsedTime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now - start_time_);
    return elapsed.count();
}

bool SystemInterface_SDL::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    const char* prefix = "RmlUi";
    switch (type) {
        case Rml::Log::LT_ERROR:   std::fprintf(stderr, "[%s ERROR] %s\n", prefix, message.c_str()); break;
        case Rml::Log::LT_WARNING: std::fprintf(stderr, "[%s WARN]  %s\n", prefix, message.c_str()); break;
        case Rml::Log::LT_INFO:    std::fprintf(stderr, "[%s INFO]  %s\n", prefix, message.c_str()); break;
        default:                   std::fprintf(stderr, "[%s DEBUG] %s\n", prefix, message.c_str()); break;
    }
    return true;
}

void SystemInterface_SDL::SetClipboardText(const Rml::String& text) {
    clipboard_ = text;
    SDL_SetClipboardText(text.c_str());
}

void SystemInterface_SDL::GetClipboardText(Rml::String& text) {
    char* clip = SDL_GetClipboardText();
    if (clip) {
        text = clip;
        SDL_free(clip);
    } else {
        text = clipboard_;
    }
}

} // namespace exd::app::rmlui
