#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

const float SPEED = 2.5f;
const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

enum Action {
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
};

class Camera {
private:
    // camera position
    glm::vec3 pos;
    glm::vec3 target;
    // unit vector at which the camera is looking at
    glm::vec3 view_direction;
    // camera up direction
    glm::vec3 up;
    // camera right direction
    glm::vec3 right;

    glm::mat4 view_matrix;

public:
    Camera(glm::vec3 _pos, glm::vec3 _target) {
        target = _target;
        pos = _pos;
        view_matrix = glm::mat4(0.0f);
        update_vectors();
    }

    void update_vectors() {
        view_direction = glm::normalize(target - pos);
        right = glm::normalize(glm::cross(WORLD_UP, view_direction));
        up = glm::normalize(glm::cross(view_direction, right));
        view_matrix = glm::lookAt(pos, target, up);
    }

    void handle_key_action(Action action, float dt) {
        // TODO: compute dt properly with fps
        switch (action) {
        case FORWARD:
            pos += view_direction * SPEED * dt;
            break;
        case BACKWARD:
            pos -= view_direction * SPEED * dt;
            break;
        case RIGHT:
            pos += right * SPEED * dt;
            break;
        case LEFT:
            pos -= right * SPEED * dt;
            break;
        }
        update_vectors();
    }

    void handle_mouse_action(int xpos, int ypos) {
        // TODO: should be view_direction, but since target is at origin 
        // so pos = view_direction 
        pos = glm::rotateY(pos, glm::radians((float) -xpos*0.2f));
        pos = glm::rotateX(pos, glm::radians((float) -ypos*0.2f));
        update_vectors();
    }

    glm::vec3 get_pos() { return pos; }
    glm::mat4 get_view_matrix() { return view_matrix; }
};
