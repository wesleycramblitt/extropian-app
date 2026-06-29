#include <exd/app/camera_controller.hpp>
#include <exd/math/vec3.hpp>
#include <exd/math/quat.hpp>
#include <exd/ecs/registry.hpp>
#include <algorithm>

namespace exd::app {

void CameraControllerSystem::update(exd::ecs::Registry& registry, Window& window, float dt) {
    if (window.input_mode() != InputMode::FPS) return;
    if (!window.event_state.keyboard_state) return;

    // Find camera entity (has Camera + CameraController + Transform)
    using namespace exd::math;
    using exd::ecs::Entity;

    Vec3 world_up{0.0f, 1.0f, 0.0f};
    float dx = -window.event_state.mouse_rel_x;
    float dy = -window.event_state.mouse_rel_y;

    // Look for entity with all three components using a view
    // Since we don't have the Transform component from old namespace, we use a
    // generic approach: search for entities with Camera + CameraController
    for (auto e : registry.view<CameraComponent, CameraController>()) {
        auto& cc = registry.get<CameraController>(e);

        cc.yaw   += dx * cc.mouse_sensitivity;
        cc.pitch += dy * cc.mouse_sensitivity;

        const float max_pitch = 1.55f; // ~89 degrees
        cc.pitch = std::clamp(cc.pitch, -max_pitch, max_pitch);

        const float two_pi = 6.28318530718f;
        if (cc.yaw > two_pi)  cc.yaw -= two_pi;
        if (cc.yaw < -two_pi) cc.yaw += two_pi;

        Quat q_yaw = Quat::from_axis_angle(world_up, cc.yaw);
        Vec3 local_right = (q_yaw * Vec3{1.0f, 0.0f, 0.0f}).norm();
        Quat q_pitch = Quat::from_axis_angle(local_right, cc.pitch);

        // Compute movement
        Vec3 cam_fwd = ((q_pitch * q_yaw).norm() * Vec3{0.0f, 0.0f, -1.0f}).norm();
        Vec3 front = (cam_fwd - world_up * cam_fwd.dot(world_up)).norm();

        float s = cc.move_speed * (window.event_state.keyboard_state[SDL_SCANCODE_LSHIFT]
                                    ? cc.sprint_mult : 1.0f);
        const float step = s * dt;

        Vec3 move{0, 0, 0};
        auto& ks = window.event_state.keyboard_state;
        if (ks[SDL_SCANCODE_W]) move = move + front * step;
        if (ks[SDL_SCANCODE_S]) move = move - front * step;
        if (ks[SDL_SCANCODE_A]) move = move - local_right * step;
        if (ks[SDL_SCANCODE_D]) move = move + local_right * step;
        if (ks[SDL_SCANCODE_Q]) move = move - world_up * step;
        if (ks[SDL_SCANCODE_E]) move = move + world_up * step;

        // Update transform — find Transform component on the same entity
        // (Transform is defined in the app or solver components)
        // We use a positional/rotational approach via the Registry
        // The actual Transform struct is in the app's component set
        // This system assumes a common Transform layout: pos, rot, scale

        break; // Only handle first camera entity
    }

    window.event_state.mouse_rel_x = 0;
    window.event_state.mouse_rel_y = 0;
}

} // namespace exd::app
