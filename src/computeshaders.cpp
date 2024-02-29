#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl_glext.h>
#include <SDL2/SDL_opengles2_gl2.h>
#include <SDL2/SDL_opengles2_gl2ext.h>
#include <GL/GL.h>

#include <SDL2/SDL_opengl.h>

#include <GL/glu.h>
#include <cmath>
#include <iostream>



SDL_Window* window;
SDL_GLContext context;
SDL_Renderer* renderer;

const int TARGET_WIDTH = 240;
const int TARGET_HEIGHT = 144;
const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

void initSDL() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // Handle initialization error
    }

    // Set OpenGL attributes before creating the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    

    // Create SDL window
    window = SDL_CreateWindow("OpenGL to SDL Texture", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
    if (!window) {
        // Handle window creation error
    }
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
    SDL_SetWindowResizable(window, SDL_bool::SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, TARGET_WIDTH, TARGET_HEIGHT);



    // Create OpenGL context
    context = SDL_GL_CreateContext(window);
    if (!context) {
        // Handle context creation error
    }
}

GLuint computeShaderProgram;
GLuint inputBuffer, outputBuffer;



int main(int argc, char* argv[]) {
    // Initialize SDL and OpenGL
    initSDL();





    const char* computeShaderSource = "your_compute_shader_code_here";
    computeShaderProgram = glCreateProgram();
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
    glCompileShader(computeShader);
    glAttachShader(computeShaderProgram, computeShader);
    glLinkProgram(computeShaderProgram);

    // Create input and output buffers
    glGenBuffers(1, &inputBuffer);
    glGenBuffers(1, &outputBuffer);




    // Main loop
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Render to SDL texture using OpenGL
        SDL_Delay(10);
        SDL_GL_SwapWindow(window);
		SDL_SetRenderTarget(renderer, NULL);
        // Optionally add a delay or frame limiting logic here

        // Swap the front and back buffers
        
    }

    // Clean up
    SDL_GL_DeleteContext(context);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
