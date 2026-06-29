#include <exd/app/mode.hpp>
#include <algorithm>
#include <stdexcept>

namespace exd::app {

void ModeManager::define(Mode mode) {
    modes_.push_back(std::move(mode));
}

void ModeManager::request_transition(int target_mode_id) {
    if (is_transitioning()) return;
    pending_id_ = target_mode_id;
    transitioning_ = true;

    // Execute transition synchronously for now
    for (auto& cb : callbacks_) cb(current_id_, target_mode_id);
    current_id_ = target_mode_id;
    pending_id_ = -1;
    transitioning_ = false;
}

const Mode& ModeManager::current() const {
    for (auto& m : modes_)
        if (m.id == current_id_) return m;
    static Mode invalid{-1, "invalid", "No mode defined"};
    return invalid;
}

bool ModeManager::is_transitioning() const { return transitioning_; }

const Mode* ModeManager::find(int id) const {
    for (auto& m : modes_)
        if (m.id == id) return &m;
    return nullptr;
}

void ModeManager::on_change(std::function<void(int, int)> callback) {
    callbacks_.push_back(std::move(callback));
}

} // namespace exd::app
