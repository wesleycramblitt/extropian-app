// Data binding bridge: C++ getter/setter ↔ RmlUi element values.
//
// The binding pattern:
//   doc->bind("domain.nx", [&] { return std::to_string(domain_.nx); });
//
// Each frame, update_bindings() polls all bound getters and updates the
// corresponding RML element's inner text if the value has changed.

#include <exd/app/ui_host.hpp>
#include <string>
#include <functional>

namespace exd::app::rmlui {

/// Helper: bind a numeric value that changes frequently (sliders, etc.).
/// Usage: bind_numeric(doc, "domain.nx", domain_.nx);
template <typename T>
inline void bind_numeric(Document& doc, const std::string& id,
                          T& value, const std::string& format = "{}") {
    doc.bind(id, [&value, format]() -> std::string {
        if constexpr (std::is_floating_point_v<T>) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), format.c_str(), value);
            return buf;
        } else {
            return std::to_string(value);
        }
    });
}

} // namespace exd::app::rmlui
