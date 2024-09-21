#include <SDL2/SDL_timer.h>
#include <iostream>
#include <string>
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
#include "../include/mesh.hpp"
#include "../include/shader.hpp"

namespace fs = std::filesystem;

const fs::path this_filepath =  __FILE__;
const fs::path ROOT = this_filepath.parent_path().parent_path();

int WIDTH = 1200;
int HEIGHT = 900;
bool QUIT = false;

SDL_Window *WINDOW = nullptr;
SDL_DisplayMode DM;
SDL_GLContext GL_CONTEXT = nullptr;

Shader shader;
Mesh mesh;
Camera camera(
    glm::vec3(1.0f, 2.0f, 2.0f), // pos of camera
    glm::vec3(0.0f, 0.0f, 0.0f)  // where camera is looking
);

// model matrix
glm::mat4 MODEL;
// projection matrix
glm::mat4 PROJ;
// camera transform matrix 
glm::mat4 VIEW;

// projection matrix parameters
float FOV = 45.0f; // field of view
float NEAR_CLIP = 0.01f; 
float FAR_CLIP = 10.0f; 

static void print_opengl_info() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
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
        std::cerr << "Cannot create window" << std::endl;
        exit(EXIT_FAILURE);
    }

    GL_CONTEXT = SDL_GL_CreateContext(WINDOW);
    if (GL_CONTEXT == nullptr) {
        std::cerr << "Cannot create opengl context, exiting" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        std::cerr << "glad is not initialized, exiting" << std::endl;
        exit(EXIT_FAILURE);
    }
    print_opengl_info();
    SDL_SetWindowFullscreen(WINDOW, SDL_FALSE);

    // Load shaders
    fs::path vertex_shader_path = ROOT / "shaders/vertex_shader.glsl";
    fs::path fragment_shader_path = ROOT / "shaders/fragment_shader.glsl";
    if(!fs::exists(vertex_shader_path) || !fs::exists(fragment_shader_path)){
        std::cerr << "Could not find shaders path" << std::endl;
        exit(EXIT_FAILURE);
    }
    shader = Shader(vertex_shader_path.c_str(), fragment_shader_path.c_str());
}

void handle_input() {
    SDL_Event event;
    static int xpos = WIDTH / 2;
    static int ypos = HEIGHT / 2;
    static bool clicked = false;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT){
            QUIT = true;
            return;
        }
        else if (event.type == SDL_DROPFILE){
            std::cout << event.drop.file << std::endl;
            mesh = Mesh(std::string(event.drop.file));
        }
        else if (event.type == SDL_KEYDOWN){
            camera.handle_key_action(event.key.keysym.sym, 0.05f);
        }
        else if (event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN)
            clicked = (event.type ==  SDL_MOUSEBUTTONDOWN);

        else if (event.type == SDL_MOUSEMOTION){
            if (!clicked) continue;
            xpos = event.motion.xrel;
            ypos = event.motion.yrel;
            camera.handle_mouse_action(xpos, ypos);
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
    MODEL = mesh.get_model_matrix();
    PROJ  = glm::perspective(
        glm::radians(FOV), (float) WIDTH / (float) HEIGHT, NEAR_CLIP, FAR_CLIP);
    VIEW = camera.get_view_matrix();

    glm::vec3 light_source1 = glm::vec3(0.0f, 2.0f, 3.0f);
    glm::vec3 light_source2 = -1 * light_source1;

    shader.set_uniform("u_Proj", PROJ);
    shader.set_uniform("u_Model", MODEL);
    shader.set_uniform("u_View", VIEW);

    shader.set_uniform("u_LightSource1", light_source1);
    shader.set_uniform("u_LightSource2", light_source2);
}

void main_loop() {
    uint64_t start, end, count = 0;
    float elapsed;
    while (!QUIT) {
        start = SDL_GetPerformanceCounter();
        handle_input();
        pre_draw();
        mesh.draw(shader);
        SDL_GL_SwapWindow(WINDOW);
        // print FPS every every 5 rounds
        if (count == 5){
            end = SDL_GetPerformanceCounter();
            elapsed = (end-start)/ (float) SDL_GetPerformanceFrequency();
            std::cout << "FPS: "<< std::to_string(1.0f/elapsed) << std::endl;
            count = 0;
        }
        count += 1;
    }
}

void cleanup() {
    SDL_DestroyWindow(WINDOW);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    initialize_program();
    // read files from command line
    if (argc > 1) {
        mesh = Mesh(std::string(argv[1]));
    }
    main_loop();
    cleanup();
    return 0;
}
