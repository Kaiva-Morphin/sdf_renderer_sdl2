#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
using namespace glm;

// Function to handle SDL errors
void sdlError(const std::string& errorMessage) {
    std::cerr << errorMessage << " Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    exit(1);
}

GLuint tilesetTexture;
ivec2 texture_size;

void renderTextureToScreen(GLuint textureID, int srcX, int srcY, int srcW, int srcH, int destX, int destY, float destW, float destH) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    glTexCoord2f((float)srcX / (float)texture_size.x, (float)srcY / (float)texture_size.y); glVertex2f(destX, destY);
    glTexCoord2f((float)(srcX + srcW) / (float)texture_size.x, (float)srcY / (float)texture_size.y); glVertex2f(destX + destW, destY);

    glTexCoord2f((float)(srcX + srcW) / (float)texture_size.x, (float)(srcY + srcH) / (float)texture_size.y); glVertex2f(destX + destW, destY + destH);
    glTexCoord2f((float)srcX / (float)texture_size.x, (float)(srcY + srcH) / (float)texture_size.y); glVertex2f(destX, destY + destH);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}



void initTexture() {
    // Load tileset image and create texture
    SDL_Surface* surface = IMG_Load("assets/better_tiles.png");
    if (!surface) {
        std::cerr << "Failed to load tileset image: " << SDL_GetError() << std::endl;
        // Handle error
        return;
    }

    glGenTextures(1, &tilesetTexture);
    glBindTexture(GL_TEXTURE_2D, tilesetTexture);
    texture_size = {surface->w, surface->h};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    SDL_FreeSurface(surface);
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        sdlError("SDL initialization failed");
    }
    IMG_Init(IMG_INIT_PNG);
    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("OpenGL with SDL2 and GLEW",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_OPENGL);
    
    if (!window) {
        sdlError("Window creation failed");
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Create OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        sdlError("OpenGL context creation failed");
    }

    // Initialize GLEW
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(glewError) << std::endl;
        exit(1);
    }

    // Check OpenGL version
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    initTexture();
    // Set up OpenGL viewport
    glViewport(0, 0, 800, 600);

    // Main loop flag
    bool quit = false;
    SDL_Event event;

    // Main loop
    while (!quit) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render a simple triangle
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);   // Red
        glVertex3f(-0.5f, -0.5f, 0.0f);
        glColor3f(0.0f, 1.0f, 0.0f);   // Green
        glVertex3f(0.5f, -0.5f, 0.0f);
        glColor3f(0.0f, 0.0f, 1.0f);   // Blue
        glVertex3f(0.0f, 0.5f, 0.0f);
        glEnd();
        glColor3f(1., 1., 1.);
        renderTextureToScreen(tilesetTexture, 0, 0, 16, 16, 0, 0, 0.5, 0.5);

        
        glBegin(GL_POINTS);
            glColor3f(1,1,1);
            glVertex2i(100,100);
        glEnd();


        // Swap buffers
        SDL_GL_SwapWindow(window);

    }

    // Clean up
    IMG_Quit();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}