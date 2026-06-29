#include <exd/app/system_graph.hpp>
#include <exd/ecs/system.hpp>
#include <exd/ecs/registry.hpp>
#include <algorithm>
#include <stdexcept>

namespace exd::app {

SystemGraph& SystemGraph::always() {
    if (last_added_ < 0 || last_added_ >= (int)entries_.size()) return *this;
    entries_[last_added_].always_active = true;
    entries_[last_added_].active_modes.clear();
    return *this;
}

SystemGraph& SystemGraph::in_mode(int mode_id) {
    if (last_added_ < 0 || last_added_ >= (int)entries_.size()) return *this;
    entries_[last_added_].always_active = false;
    entries_[last_added_].active_modes.insert(mode_id);
    return *this;
}

SystemGraph& SystemGraph::in_modes(std::set<int> mode_ids) {
    if (last_added_ < 0 || last_added_ >= (int)entries_.size()) return *this;
    entries_[last_added_].always_active = false;
    entries_[last_added_].active_modes = std::move(mode_ids);
    return *this;
}

void SystemGraph::build() {
    // Sort: always-active first, then mode-specific
    std::stable_sort(entries_.begin(), entries_.end(),
        [](const Entry& a, const Entry& b) { return a.always_active && !b.always_active; });
}

void SystemGraph::update(exd::ecs::Registry& registry, double dt, int current_mode) {
    for (auto& entry : entries_) {
        bool should_run = entry.always_active ||
                          entry.active_modes.find(current_mode) != entry.active_modes.end();
        if (should_run && entry.system)
            entry.system->update(registry, dt);
    }
}

void SystemGraph::order_before(const std::string& before, const std::string& after) {
    // Simple reordering by name (systems should have names via ISystem::name())
    (void)before; (void)after; // TODO: implement if needed
}

} // namespace exd::app
