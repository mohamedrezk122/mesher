#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_video.h>
#include <glm/detail/type_vec.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/camera.hpp"
#include "../include/glad.h"
#include "../include/model.hpp"
#include "../include/shader.hpp"

namespace fs = std::filesystem;

int WIDTH = 1200;
int HEIGHT = 900;

SDL_Window *WINDOW = nullptr;
SDL_DisplayMode DM;
SDL_GLContext GL_CONTEXT = nullptr;
bool QUIT = false;

Shader shader;
Model model;
Camera camera(glm::vec3(1.0f, 2.0f, 2.0f), // pos of camera
              glm::vec3(0.0f, 0.0f, 0.0f)  // where camera is looking
);

void print_opengl_info() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading language version: "
              << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void initialize_program() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Cannot initialize SDL, exiting" << std::endl;
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    WINDOW = SDL_CreateWindow("mesher", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (WINDOW == nullptr) {
        std::cout << "Cannot create window" << std::endl;
        exit(EXIT_FAILURE);
    }

    GL_CONTEXT = SDL_GL_CreateContext(WINDOW);
    if (GL_CONTEXT == nullptr) {
        std::cout << "Cannot create opengl context, exiting" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        std::cout << "glad is not initialized, exiting" << std::endl;
        exit(EXIT_FAILURE);
    }
    print_opengl_info();
    SDL_SetWindowFullscreen(WINDOW, SDL_FALSE);

    // Load shaders
    fs::path this_filepath =  __FILE__;
    fs::path project_path = this_filepath.parent_path().parent_path();
    fs::path vertex_shader_path = project_path / "shaders/vertex_shader.glsl";
    fs::path fragment_shader_path = project_path / "shaders/fragment_shader.glsl";
    if(!fs::exists(vertex_shader_path) || !fs::exists(fragment_shader_path)){
        std::cerr << "Could not find shaders path" << std::endl;
        exit(1);
    }
    shader = Shader(vertex_shader_path.c_str(), fragment_shader_path.c_str());
}

void handle_input() {
    SDL_Event event;
    static int xpos = WIDTH / 2;
    static int ypos = HEIGHT / 2;
    static bool clicked = false;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT:
            QUIT = true;
            return;
        case SDL_DROPFILE:
            std::cout << event.drop.file << std::endl;
            model = Model(std::string(event.drop.file));
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                camera.handle_key_action(LEFT, 0.1f);
                break;
            case SDLK_RIGHT:
                camera.handle_key_action(RIGHT, 0.1f);
                break;
            case SDLK_UP:
                camera.handle_key_action(FORWARD, 0.1f);
                break;
            case SDLK_DOWN:
                camera.handle_key_action(BACKWARD, 0.1f);
                break;
            default:
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            clicked = true;
            break;
        case SDL_MOUSEBUTTONUP:
            clicked = false;
            break;
        case SDL_MOUSEMOTION:
            if (!clicked)
                break;
            xpos = event.motion.xrel;
            ypos = event.motion.yrel;
            camera.handle_mouse_action(xpos, ypos);
            break;
        default:
            break;
        }
    }
}

void pre_draw() {

    SDL_GetCurrentDisplayMode(0, &DM);
    int WIDTH = DM.w;
    int HEIGHT = DM.h;

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // set the winding
    glFrontFace(GL_CCW);

    glViewport(0, 0, WIDTH, HEIGHT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT, GL_LINE);
    glPolygonMode(GL_BACK, GL_LINE);

    shader.use();
    glm::mat4 model_matrix = model.get_model_matrix();
    glm::mat4 proj_matrix = glm::perspective(
        glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 10.0f);

    glm::mat4 view_matrix = camera.get_view_matrix();
    glm::vec3 light_source_pos1 = glm::vec3(0.0f, 2.0f, 3.0f);
    glm::vec3 light_source_pos2 = -1 * light_source_pos1;

    shader.set_uniform("u_Proj", proj_matrix);
    shader.set_uniform("u_Model", model_matrix);
    shader.set_uniform("u_View", view_matrix);
    shader.set_uniform("u_LightSource1", light_source_pos1);
    shader.set_uniform("u_LightSource2", light_source_pos2);
}

void main_loop() {
    while (!QUIT) {
        handle_input();
        pre_draw();
        model.draw(shader);
        SDL_GL_SwapWindow(WINDOW);
    }
}

void cleanup() {
    SDL_DestroyWindow(WINDOW);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    initialize_program();
    // read files from command lines
    if (argc > 1) {
        model = Model(std::string(argv[1]));
    }
    main_loop();
    cleanup();
    return 0;
}
