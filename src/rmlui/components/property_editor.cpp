// Reusable PropertyEditor component — C++ side.
// The visual layout is defined in assets/panels/property_editor.rml.
// This file provides a C++ API for populating the property grid.

#include <exd/app/ui_host.hpp>
#include <string>
#include <vector>
#include <functional>

namespace exd::app::rmlui {

struct PropertyDef {
    std::string label;
    std::string id;
    std::function<std::string()> getter;
};

/// Register a set of properties on a document's property editor panel.
/// The document must contain a <property-grid> element.
inline void setup_property_editor(Document& doc, const std::vector<PropertyDef>& props) {
    for (const auto& p : props) {
        doc.bind(p.id, p.getter);
    }
}

} // namespace exd::app::rmlui
