#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_timer.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_set>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <glm/detail/type_vec.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/bvh.hpp"
#include "../include/camera.hpp"
#include "../include/context.hpp"
#include "../include/glad.h"
#include "../include/mesh.hpp"
#include "../include/shader.hpp"

namespace fs = std::filesystem;

const fs::path this_filepath = __FILE__;
const fs::path ROOT = this_filepath.parent_path().parent_path();

Context ctx;

Shader shader;
std::vector<Mesh> triangles;
// prune duplicates from selected triangles 
std::unordered_set<uint32_t> tris_idxs; 

BVH bvh;
Camera camera(glm::vec3(1.0f, 2.0f, 2.0f), // pos of camera
              glm::vec3(0.0f, 0.0f, 0.0f)  // where camera is looking
);

// model matrix
glm::mat4 MODEL;
// projection matrix
glm::mat4 PROJ;
// camera transform matrix
glm::mat4 VIEW;

// projection matrix parameters
#define FOV 45.0f // field of view
#define NEAR_CLIP 0.01f
#define FAR_CLIP 1000.0f

static void print_opengl_info() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << std::endl;
}

void initialize_program() {
    ctx.create_window(1200, 900);
    print_opengl_info();
    // Load shaders
    fs::path vertex_shader_path = ROOT / "shaders/vertex_shader.glsl";
    fs::path fragment_shader_path = ROOT / "shaders/fragment_shader.glsl";
    if (!fs::exists(vertex_shader_path) || !fs::exists(fragment_shader_path)) {
        std::cerr << "Could not find shaders path" << std::endl;
        exit(EXIT_FAILURE);
    }
    shader = Shader(vertex_shader_path.c_str(), fragment_shader_path.c_str());
}

void handle_input() {
    using namespace std::chrono;
    SDL_Event event;
    // clicked, is_left
    static auto mouse_state = std::make_pair<bool, bool>(false, 0);
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            ctx.is_quit = true;
            return;
        } else if (event.type == SDL_DROPFILE) {
            std::cout << event.drop.file << std::endl;
            mesh = Mesh(std::string(event.drop.file));
            bvh = BVH(mesh);
        } else if (event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_q){
                triangles.clear();
                tris_idxs.clear();
                continue;
            }
            camera.handle_key_action(event.key.keysym.sym, 0.05f);
        } else if (event.type == SDL_MOUSEBUTTONUP ||
                   event.type == SDL_MOUSEBUTTONDOWN) {
            mouse_state.first = (event.type == SDL_MOUSEBUTTONDOWN);
            mouse_state.second = event.button.button == SDL_BUTTON_LEFT;
        } else if (event.type == SDL_MOUSEMOTION) {
            if (!mouse_state.first)
                continue;
            // right is clicked
            if (!mouse_state.second) {
                int xpos = event.motion.xrel;
                int ypos = event.motion.yrel;
                camera.handle_mouse_action(xpos, ypos);
                continue;
            }
            auto triangle_opt =
                check_intersection(glm::vec2(event.motion.x, event.motion.y),
                                   ctx.get_viewport(), bvh, VIEW, PROJ);
            if (triangle_opt.has_value()) {
                uint32_t idx = triangle_opt.value();
                if (tris_idxs.find(idx) == tris_idxs.end()){
                    triangles.push_back(mesh.highlight_triangle(idx));
                    tris_idxs.insert(idx);
                }
            }
        }
    }
}

void pre_draw() {
    ctx.update_window();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // set the winding
    glFrontFace(GL_CCW);

    glViewport(0, 0, ctx.width, ctx.height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT, GL_LINE);
    glPolygonMode(GL_BACK, GL_LINE);

    shader.use();
    MODEL = mesh.get_model_matrix();
    PROJ = glm::perspective(glm::radians(FOV),
                            (float)ctx.width / (float)ctx.height, NEAR_CLIP,
                            FAR_CLIP);
    VIEW = camera.get_view_matrix();

    glm::vec3 light_source1 = glm::vec3(0.0f, 2.0f, 3.0f);
    glm::vec3 light_source2 = -1 * light_source1;

    shader.set_uniform("u_Proj", PROJ);
    shader.set_uniform("u_Model", MODEL);
    shader.set_uniform("u_View", VIEW);

    shader.set_uniform("u_LightSource1", light_source1);
    shader.set_uniform("u_LightSource2", light_source2);
    shader.set_uniform("u_LightSource2", light_source2);
}

void main_loop() {
    // uint64_t start, end, count = 0;
    // float elapsed;
    // mesh_box = Mesh_bounding_box(mesh);
    while (!ctx.is_quit) {
        // start = SDL_GetPerformanceCounter();
        handle_input();
        pre_draw();
        mesh.draw(shader);
        for(auto& tri: triangles)
            tri.draw(shader);
        SDL_GL_SwapWindow(ctx.window);
        // print FPS every every 5 rounds
        // if (count == 5){
        //     end = SDL_GetPerformanceCounter();
        //     elapsed = (end-start)/ (float) SDL_GetPerformanceFrequency();
        //     // std::cout << "FPS: "<< std::to_string(1.0f/elapsed) <<
        //     std::endl; count = 0;
        // }
        // count += 1;
    }
}

int main(int argc, char *argv[]) {
    initialize_program();
    // read files from command line
    if (argc > 1) {
        mesh = Mesh(std::string(argv[1]));
        mesh_box = mesh.construct_bounding_box();
        bvh = BVH(mesh);
        std::cout << mesh.triangles.size() << std::endl;
    }
    main_loop();
    return 0;
}
