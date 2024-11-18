#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <cassert>
#include <glm/vec4.hpp>
#include <iostream>

#include "../include/glad.h"

class Context {
  private:
    SDL_DisplayMode dm;

  public:
    int width, height;
    bool is_quit = false;

    SDL_Window *window = nullptr;
    SDL_GLContext gl_context = nullptr;

    Context() = default;

    glm::vec4 get_viewport() { return glm::vec4(0.0f, 0.0f, width, height); }

    void initialize_sdl() {
        assert(SDL_Init(SDL_INIT_VIDEO) >= 0 && "Cannot initialize SDL");

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
    }

    void initialize_opengl() {
        gl_context = SDL_GL_CreateContext(window);
        assert(gl_context != nullptr && "Cannot create opengl context");
        assert(gladLoadGLLoader(SDL_GL_GetProcAddress) &&
               "Cannot initialize glad");
    }

    void create_window(int _width, int _height) {
        width = _width;
        height = _height;
        initialize_sdl();
        window = SDL_CreateWindow("mesher", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, width, height,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        assert(window != nullptr && "Cannot create window");
        SDL_SetWindowFullscreen(window, SDL_FALSE);
        initialize_opengl();
    }

    void update_window() {
        SDL_GetCurrentDisplayMode(0, &dm);
        width = dm.w;
        height = dm.h;
    }

    ~Context() {
        std::cout << "I am leaving" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};