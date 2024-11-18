#pragma once

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#define SPEED 2.5f
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)

class Camera {
  private:
    glm::vec3 target;
    // unit vector at which the camera is looking at
    glm::vec3 view_direction;
    // camera up direction
    glm::vec3 up;
    // camera right direction
    glm::vec3 right;
    glm::mat4 view_matrix;

  public:
    // camera position
    glm::vec3 pos;

    Camera(glm::vec3 _pos, glm::vec3 _target);
    void update_vectors();
    void handle_key_action(SDL_Keycode action, float dt);
    void handle_mouse_action(int xpos, int ypos);
    void reset_to_x();
    void reset_to_y();
    void reset_to_z();
    glm::vec3 get_pos();
    glm::mat4 get_view_matrix();
};
