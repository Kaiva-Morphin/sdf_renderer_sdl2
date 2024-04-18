#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;

    uniform sampler2D texture1;

    void main() {
        FragColor = texture(texture1, TexCoord);
    }
)";

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    // Set OpenGL attributes before creating the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    
    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    glewInit();

    glEnable(GL_TEXTURE_2D);
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 20, 20, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "err" << std::endl;
    }

    
    // Render loop
    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
            // Render triangle to texture
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, 20, 20); // Set viewport to match texture size
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(0.f, 0.f, 0.0f);
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.5f, 0.5f, 0.0f);
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3f(0.0f, 0.5f, 0.0f);
            glEnd();

            // Render texture to screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, 800, 600); // Set viewport to match screen size
            glClearColor(1.f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindTexture(GL_TEXTURE_2D, texture);
            glColor3f(1.0f, 1.0f, 1.0f); 
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f); 
            glEnd();

            SDL_GL_SwapWindow(window);
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &fbo);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
