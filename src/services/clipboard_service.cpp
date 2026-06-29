#include <string>
#include <SDL3/SDL.h>
namespace exd::app::services {
void clipboard_set(const std::string& text) { SDL_SetClipboardText(text.c_str()); }
std::string clipboard_get() {
    char* c = SDL_GetClipboardText();
    std::string s(c ? c : "");
    if (c) SDL_free(c);
    return s;
}
}
