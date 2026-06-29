#pragma once

#include <exd/ecs/registry.hpp>
#include <exd/app/window.hpp>

namespace exd::app {

struct CameraComponent {
    float fov_y_radians = 1.047f; // 60 degrees
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
};

struct CameraController {
    float move_speed = 30.0f;
    float sprint_mult = 2.0f;
    float mouse_sensitivity = 0.002f;
    float yaw = 0.0f;
    float pitch = 0.0f;
};

/// FPS-style camera controller. Reads keyboard/mouse from Window::event_state
/// and updates the camera entity's Transform component.
class CameraControllerSystem {
public:
    void update(exd::ecs::Registry& registry, Window& window, float dt);
};

} // namespace exd::app
