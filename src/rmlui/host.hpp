#pragma once

#include <exd/app/ui_host.hpp>
#include "render_interface.hpp"
#include "system_interface.hpp"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Factory.h>
// RmlUi Controls merged into Core in master

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace exd::app::rmlui {

/// @brief RmlUi implementation of IUIHost.
///
/// Manages RmlUi contexts, loads documents, processes input events,
/// and renders UI each frame through the OpenGL render interface.
class RmlUiHost : public IUIHost {
public:
    explicit RmlUiHost(void* sdl_window_handle);
    ~RmlUiHost() override;

    // ── IUIHost interface ────────────────────────────
    std::unique_ptr<Document> load_document(const std::string& path) override;
    void close_document(Document* doc) override;
    void begin_frame() override;
    void end_frame() override;
    void load_stylesheet(const std::string& path) override;

    // ── Input forwarding (called by app main loop) ───
    void process_sdl_event(const SDL_Event& event);
    void set_viewport_size(int width, int height);

    // ── Access ───────────────────────────────────────
    RenderInterface_GL& render_interface() { return render_interface_; }

private:
    void setup_rmlui();

    RenderInterface_GL render_interface_;
    SystemInterface_SDL system_interface_;
    Rml::Context* context_ = nullptr;
    void* sdl_window_ = nullptr;
    bool initialized_ = false;

    // Active documents
    std::vector<std::unique_ptr<Document>> documents_;

    // Key modifier state for RmlUi
    int key_modifiers_ = 0;

    // Map SDL key codes → RmlUi key identifiers
    static Rml::Input::KeyIdentifier sdl_key_to_rml(SDL_Keycode key);
    static int sdl_mod_to_rml(SDL_Keymod mod);
    static int sdl_button_to_rml(int button);
};

/// @brief RmlUi Document — wraps Rml::ElementDocument.
class RmlDocument : public Document {
public:
    explicit RmlDocument(Rml::ElementDocument* doc) : doc_(doc) {
        doc_->Show();
    }
    ~RmlDocument() override {
        if (doc_) doc_->Close();
    }

    void bind(const std::string& model_path,
              std::function<std::string()> getter) override;
    void on(const std::string& event_name, EventCallback callback) override;
    void set_inner_rml(const std::string& element_id,
                       const std::string& rml) override;
    void set_attribute(const std::string& element_id,
                       const std::string& attr,
                       const std::string& value) override;
    void show() override;
    void hide() override;
    [[nodiscard]] bool is_visible() const override;
    void update() override;

    /// Flush all data bindings (call each frame).
    void update_bindings();

    /// Forward events to registered callbacks.
    void dispatch_event(const std::string& event_name);

    [[nodiscard]] Rml::ElementDocument* raw() { return doc_; }

private:
    Rml::ElementDocument* doc_;
    std::unordered_map<std::string, std::function<std::string()>> bindings_;
    std::unordered_map<std::string, std::string> cached_values_;
    std::unordered_map<std::string, std::vector<EventCallback>> event_callbacks_;
};

} // namespace exd::app::rmlui
