#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>

namespace exd::app {

class Document;

/// @brief Abstract UI host — the top-level interface for all UI backends.
///
/// Implemented by RmlUi (primary), ImGui (fallback), and future Qt6 backends.
/// Application code writes against this interface and never cares which backend
/// is active.
class IUIHost {
public:
    virtual ~IUIHost() = default;

    /// ── Document lifecycle ────────────────────────────
    /// Load an RML document. Returns a handle for data binding and events.
    virtual std::unique_ptr<Document> load_document(const std::string& path) = 0;
    virtual void close_document(Document* doc) = 0;

    /// ── Frame lifecycle ───────────────────────────────
    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;

    /// ── Style ─────────────────────────────────────────
    virtual void load_stylesheet(const std::string& path) = 0;

    /// ── Factory ────────────────────────────────────────
    enum class Backend { RmlUi, ImGui, Qt6, Null };
    static std::unique_ptr<IUIHost> create(Backend backend, void* window_handle);
};

/// @brief A loaded UI document. Supports data binding and event handling.
class Document {
public:
    virtual ~Document() = default;

    /// ── Data binding ──────────────────────────────────
    /// Bind a C++ value to an RML data-model path.
    /// Usage: doc->bind("domain.nx", [&] { return domain_.nx; });
    virtual void bind(const std::string& model_path,
                      std::function<std::string()> getter) = 0;

    /// ── Event handling ────────────────────────────────
    using EventCallback = std::function<void(const std::string& event_name)>;
    virtual void on(const std::string& event_name, EventCallback callback) = 0;

    /// ── Element access ────────────────────────────────
    virtual void set_inner_rml(const std::string& element_id,
                               const std::string& rml) = 0;
    virtual void set_attribute(const std::string& element_id,
                               const std::string& attr, const std::string& value) = 0;

    /// ── Lifecycle ─────────────────────────────────────
    virtual void show() = 0;
    virtual void hide() = 0;
    [[nodiscard]] virtual bool is_visible() const = 0;
    virtual void update() = 0;
};

} // namespace exd::app
