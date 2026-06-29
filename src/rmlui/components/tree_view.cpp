// Reusable TreeView component — C++ side.
// Implements hierarchical entity/scene tree with selection.

#include <exd/app/ui_host.hpp>
#include <string>
#include <vector>
#include <functional>

namespace exd::app::rmlui {

struct TreeItem {
    std::string id;
    std::string label;
    bool is_group = false;
    bool selected = false;
    std::vector<TreeItem> children;
};

/// Build a tree view in a document by generating RML for each item.
inline void build_tree_view(Document& doc, const std::string& container_id,
                             const std::vector<TreeItem>& items) {
    std::string rml;
    for (const auto& item : items) {
        rml += "<tree-item";
        if (item.is_group) rml += " type=\"group\"";
        if (item.selected) rml += " selected=\"true\"";
        rml += " data-entity-id=\"" + item.id + "\">";
        rml += item.label;
        rml += "</tree-item>";
    }
    doc.set_inner_rml(container_id, rml);
}

} // namespace exd::app::rmlui
