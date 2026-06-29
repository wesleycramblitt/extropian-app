#include "host.hpp"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace exd::app::rmlui {

// ──────────────────────────────────────────────────────────────────────────
// RmlUiHost
// ──────────────────────────────────────────────────────────────────────────

RmlUiHost::RmlUiHost(void* sdl_window_handle)
    : sdl_window_(sdl_window_handle) {
    setup_rmlui();
}

RmlUiHost::~RmlUiHost() {
    documents_.clear();
    if (context_) {
        context_->UnloadAllDocuments();
    }
    Rml::Shutdown();
}

void RmlUiHost::setup_rmlui() {
    if (initialized_) return;

    // Initialize RmlUi with our interfaces
    Rml::SetRenderInterface(&render_interface_);
    Rml::SetSystemInterface(&system_interface_);
    Rml::Initialise();

    // Load the Controls plugin (provides standard widgets)
    Rml::Controls::Initialise();

    // Create the main context
    Rml::Vector2i size(1280, 720);
    if (sdl_window_) {
        int w, h;
        SDL_GetWindowSize(static_cast<SDL_Window*>(sdl_window_), &w, &h);
        if (w > 0 && h > 0) {
            size.x = w;
            size.y = h;
        }
    }
    context_ = Rml::CreateContext("main", size);
    if (!context_) {
        std::fprintf(stderr, "[RmlUiHost] Failed to create RmlUi context\n");
        return;
    }

    // Load base stylesheet
    context_->LoadDocument("assets/app_shell.rml");

    initialized_ = true;
    std::printf("[RmlUiHost] Initialized with context %dx%d\n", size.x, size.y);
}

std::unique_ptr<Document> RmlUiHost::load_document(const std::string& path) {
    if (!context_) {
        std::fprintf(stderr, "[RmlUiHost] No context available\n");
        return nullptr;
    }

    Rml::ElementDocument* rml_doc = context_->LoadDocument(path);
    if (!rml_doc) {
        std::fprintf(stderr, "[RmlUiHost] Failed to load document: %s\n", path.c_str());
        return nullptr;
    }

    auto doc = std::make_unique<RmlDocument>(rml_doc);
    auto* raw = doc.get();
    documents_.push_back(std::move(doc));
    return std::unique_ptr<Document>(raw, [](Document*) {
        // Ownership stays in documents_ vector
    });
}

void RmlUiHost::close_document(Document* doc) {
    auto* rml_doc = static_cast<RmlDocument*>(doc);
    rml_doc->hide();
    auto it = std::find_if(documents_.begin(), documents_.end(),
        [doc](const auto& d) { return d.get() == doc; });
    if (it != documents_.end()) {
        documents_.erase(it);
    }
}

void RmlUiHost::load_stylesheet(const std::string& path) {
    Rml::Factory::InstanceStyleSheetFile(path);
}

void RmlUiHost::begin_frame() {
    if (!context_) return;

    // Update all loaded documents
    for (auto& doc : documents_) {
        doc->update();
    }

    // Update the context (processes events, layout, animations)
    context_->Update();

    // Begin rendering
    render_interface_.begin_frame();
}

void RmlUiHost::end_frame() {
    if (!context_) return;

    // Render the context — calls our RenderInterface with geometry batches
    context_->Render();

    render_interface_.end_frame();

    // Update data bindings after render so next frame sees fresh values
    for (auto& doc : documents_) {
        static_cast<RmlDocument*>(doc.get())->update_bindings();
    }
}

void RmlUiHost::set_viewport_size(int width, int height) {
    render_interface_.set_viewport(width, height);
    if (context_) {
        context_->SetDimensions(Rml::Vector2i(width, height));
    }
}

void RmlUiHost::process_sdl_event(const SDL_Event& event) {
    if (!context_) return;

    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
            context_->ProcessMouseMove(event.motion.x, event.motion.y, key_modifiers_);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            int button = sdl_button_to_rml(event.button.button);
            bool down = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
            if (down)
                context_->ProcessMouseButtonDown(button, key_modifiers_);
            else
                context_->ProcessMouseButtonUp(button, key_modifiers_);
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL:
            context_->ProcessMouseWheel(event.wheel.y, key_modifiers_);
            break;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            key_modifiers_ = sdl_mod_to_rml(SDL_GetModState());
            auto rml_key = sdl_key_to_rml(event.key.key);
            bool down = (event.type == SDL_EVENT_KEY_DOWN);

            // Handle text input for printable characters
            if (down && event.key.key >= SDLK_SPACE && event.key.key <= SDLK_Z) {
                char utf8[2] = { static_cast<char>(event.key.key), 0 };
                context_->ProcessTextInput(utf8);
            }

            if (down)
                context_->ProcessKeyDown(rml_key, key_modifiers_);
            else
                context_->ProcessKeyUp(rml_key, key_modifiers_);
            break;
        }
        case SDL_EVENT_TEXT_INPUT:
            context_->ProcessTextInput(event.text.text);
            break;

        default:
            break;
    }
}

// ── Key/Button mapping helpers ─────────────────────────────────────────

Rml::Input::KeyIdentifier RmlUiHost::sdl_key_to_rml(SDL_Keycode key) {
    using KI = Rml::Input::KeyIdentifier;
    switch (key) {
        case SDLK_SPACE:       return KI::KI_SPACE;
        case SDLK_0:           return KI::KI_0;
        case SDLK_1:           return KI::KI_1;
        case SDLK_2:           return KI::KI_2;
        case SDLK_3:           return KI::KI_3;
        case SDLK_4:           return KI::KI_4;
        case SDLK_5:           return KI::KI_5;
        case SDLK_6:           return KI::KI_6;
        case SDLK_7:           return KI::KI_7;
        case SDLK_8:           return KI::KI_8;
        case SDLK_9:           return KI::KI_9;
        case SDLK_A:           return KI::KI_A;
        case SDLK_B:           return KI::KI_B;
        case SDLK_C:           return KI::KI_C;
        case SDLK_D:           return KI::KI_D;
        case SDLK_E:           return KI::KI_E;
        case SDLK_F:           return KI::KI_F;
        case SDLK_G:           return KI::KI_G;
        case SDLK_H:           return KI::KI_H;
        case SDLK_I:           return KI::KI_I;
        case SDLK_J:           return KI::KI_J;
        case SDLK_K:           return KI::KI_K;
        case SDLK_L:           return KI::KI_L;
        case SDLK_M:           return KI::KI_M;
        case SDLK_N:           return KI::KI_N;
        case SDLK_O:           return KI::KI_O;
        case SDLK_P:           return KI::KI_P;
        case SDLK_Q:           return KI::KI_Q;
        case SDLK_R:           return KI::KI_R;
        case SDLK_S:           return KI::KI_S;
        case SDLK_T:           return KI::KI_T;
        case SDLK_U:           return KI::KI_U;
        case SDLK_V:           return KI::KI_V;
        case SDLK_W:           return KI::KI_W;
        case SDLK_X:           return KI::KI_X;
        case SDLK_Y:           return KI::KI_Y;
        case SDLK_Z:           return KI::KI_Z;
        case SDLK_ESCAPE:      return KI::KI_ESCAPE;
        case SDLK_RETURN:      return KI::KI_RETURN;
        case SDLK_TAB:         return KI::KI_TAB;
        case SDLK_BACKSPACE:   return KI::KI_BACK;
        case SDLK_DELETE:      return KI::KI_DELETE;
        case SDLK_INSERT:      return KI::KI_INSERT;
        case SDLK_HOME:        return KI::KI_HOME;
        case SDLK_END:         return KI::KI_END;
        case SDLK_PAGEUP:      return KI::KI_PRIOR;
        case SDLK_PAGEDOWN:    return KI::KI_NEXT;
        case SDLK_UP:          return KI::KI_UP;
        case SDLK_DOWN:        return KI::KI_DOWN;
        case SDLK_LEFT:        return KI::KI_LEFT;
        case SDLK_RIGHT:       return KI::KI_RIGHT;
        case SDLK_F1:          return KI::KI_F1;
        case SDLK_F2:          return KI::KI_F2;
        case SDLK_F3:          return KI::KI_F3;
        case SDLK_F4:          return KI::KI_F4;
        case SDLK_F5:          return KI::KI_F5;
        case SDLK_F6:          return KI::KI_F6;
        case SDLK_F7:          return KI::KI_F7;
        case SDLK_F8:          return KI::KI_F8;
        case SDLK_F9:          return KI::KI_F9;
        case SDLK_F10:         return KI::KI_F10;
        case SDLK_F11:         return KI::KI_F11;
        case SDLK_F12:         return KI::KI_F12;
        case SDLK_LSHIFT:      return KI::KI_LSHIFT;
        case SDLK_RSHIFT:      return KI::KI_RSHIFT;
        case SDLK_LCTRL:       return KI::KI_LCONTROL;
        case SDLK_RCTRL:       return KI::KI_RCONTROL;
        case SDLK_LALT:        return KI::KI_LMENU;
        case SDLK_RALT:        return KI::KI_RMENU;
        case SDLK_PAUSE:       return KI::KI_PAUSE;
        case SDLK_CAPSLOCK:    return KI::KI_CAPITAL;
        case SDLK_NUMLOCKCLEAR: return KI::KI_NUMLOCK;
        case SDLK_SCROLLLOCK:  return KI::KI_SCROLL;
        default:               return KI::KI_UNKNOWN;
    }
}

int RmlUiHost::sdl_mod_to_rml(SDL_Keymod mod) {
    int rml = 0;
    if (mod & SDL_KMOD_CTRL)  rml |= Rml::Input::KM_CTRL;
    if (mod & SDL_KMOD_SHIFT) rml |= Rml::Input::KM_SHIFT;
    if (mod & SDL_KMOD_ALT)   rml |= Rml::Input::KM_ALT;
    if (mod & SDL_KMOD_GUI)   rml |= Rml::Input::KM_META;
    if (mod & SDL_KMOD_CAPS)  rml |= Rml::Input::KM_CAPSLOCK;
    if (mod & SDL_KMOD_NUM)   rml |= Rml::Input::KM_NUMLOCK;
    return rml;
}

int RmlUiHost::sdl_button_to_rml(int button) {
    switch (button) {
        case SDL_BUTTON_LEFT:   return 0;
        case SDL_BUTTON_RIGHT:  return 1;
        case SDL_BUTTON_MIDDLE: return 2;
        default:                return button - 1;
    }
}

// ──────────────────────────────────────────────────────────────────────────
// RmlDocument
// ──────────────────────────────────────────────────────────────────────────

void RmlDocument::bind(const std::string& model_path,
                       std::function<std::string()> getter) {
    bindings_[model_path] = std::move(getter);
    cached_values_[model_path] = "";
}

void RmlDocument::update_bindings() {
    for (auto& [path, getter] : bindings_) {
        std::string value = getter();
        if (cached_values_[path] != value) {
            cached_values_[path] = value;

            // Find element by data-model attribute and update inner text
            // RmlUi doesn't have a direct query by data attribute, so we
            // use a custom attribute on the element.
            // The RML elements use: <p data-model="domain.nx">value here</p>
            // We update via element ID or attribute selector.

            // For elements with id matching the binding path:
            Rml::Element* el = doc_->GetElementById(path);
            if (el) {
                el->SetInnerRML(value);
                continue;
            }

            // For data-model attribute (custom approach):
            // We traverse all descendants and match data-model attr.
            // This is a fallback for when id doesn't match.
            // In practice, prefer using: id="domain.nx" on the element.
        }
    }
}

void RmlDocument::on(const std::string& event_name, EventCallback callback) {
    event_callbacks_[event_name].push_back(std::move(callback));

    // Register with RmlUi if this is a known event type
    // For custom events ("onSolve", "onStop", etc.), we hook into
    // the RmlUi event system via data-event attributes:
    // <button data-event-click="onSolve">Run</button>
    //
    // We listen for 'click' events on the document and check
    // for the data-event-click attribute, then dispatch.
    if (event_name == "click") {
        doc_->AddEventListener(Rml::EventId::Click, [this](Rml::Event& ev) {
            Rml::Element* target = ev.GetTargetElement();
            if (target) {
                Rml::String event_name;
                if (target->GetAttribute("data-event-click", event_name)) {
                    dispatch_event(event_name);
                }
            }
        });
    }
}

void RmlDocument::dispatch_event(const std::string& event_name) {
    auto it = event_callbacks_.find(event_name);
    if (it != event_callbacks_.end()) {
        for (auto& cb : it->second) {
            cb(event_name);
        }
    }
}

void RmlDocument::set_inner_rml(const std::string& element_id,
                                 const std::string& rml) {
    Rml::Element* el = doc_->GetElementById(element_id);
    if (el) el->SetInnerRML(rml);
}

void RmlDocument::set_attribute(const std::string& element_id,
                                 const std::string& attr,
                                 const std::string& value) {
    Rml::Element* el = doc_->GetElementById(element_id);
    if (el) el->SetAttribute(attr, value);
}

void RmlDocument::show() {
    if (doc_) doc_->Show();
}

void RmlDocument::hide() {
    if (doc_) doc_->Hide();
}

bool RmlDocument::is_visible() const {
    return doc_ && doc_->IsVisible();
}

void RmlDocument::update() {
    // Update bindings each frame (called by RmlUiHost::begin_frame → update)
    update_bindings();

    // Process pending document updates
    if (doc_) {
        doc_->UpdateDocument();
    }
}

} // namespace exd::app::rmlui
