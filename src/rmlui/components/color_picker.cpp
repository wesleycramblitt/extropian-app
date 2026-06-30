// ColorPicker component — C++ stub.
#include <string>
// In the future: HSV color wheel with alpha slider.

namespace exd::app::rmlui {

struct Color {
    float r = 1, g = 1, b = 1, a = 1;
};

inline void show_color_picker(const std::string& element_id,
                               Color& color) {
    // TODO: Implement color picker widget
    (void)element_id;
    (void)color;
}

} // namespace exd::app::rmlui
