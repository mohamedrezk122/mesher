#include "camera.hpp"

Camera::Camera(glm::vec3 _pos, glm::vec3 _target) {
    target = _target;
    pos = _pos;
    view_matrix = glm::mat4(0.0f);
    update_vectors();
}

void Camera::update_vectors() {
    view_direction = glm::normalize(target - pos);
    right = glm::normalize(glm::cross(WORLD_UP, view_direction));
    up = glm::normalize(glm::cross(view_direction, right));
    view_matrix = glm::lookAt(pos, target, up);
}

void Camera::handle_key_action(SDL_Keycode action, float dt) {
    // TODO: compute dt properly with fps
    switch (action) {
    case SDLK_UP:
        pos += view_direction * SPEED * dt;
        break;
    case SDLK_DOWN:
        pos -= view_direction * SPEED * dt;
        break;
    case SDLK_RIGHT:
        pos += right * SPEED * dt;
        break;
    case SDLK_LEFT:
        pos -= right * SPEED * dt;
        break;
    case SDLK_y:
        reset_to_y();
        break;
    case SDLK_x:
        reset_to_x();
        break;
    case SDLK_z:
        reset_to_z();
        break;
    default:
        break;
    }
    update_vectors();
}

void Camera::handle_mouse_action(int xpos, int ypos) {
    // TODO: should be view_direction, but since target is at origin
    // so pos = view_direction
    pos = glm::rotateY(pos, glm::radians((float)-xpos * 0.2f));
    pos = glm::rotateX(pos, glm::radians((float)-ypos * 0.2f));
    // std::cout << glm::to_string(pos) << std::endl;
    update_vectors();
}

void Camera::reset_to_x() { pos = glm::vec3(4.0f, 0.0f, 0.0f); }

void Camera::reset_to_y() { pos = glm::vec3(0.76f, 4.0f, 0.0f); }

void Camera::reset_to_z() { pos = glm::vec3(0.0f, 0.0f, 4.0f); }

glm::vec3 Camera::get_pos() { return pos; }
glm::mat4 Camera::get_view_matrix() { return view_matrix; }