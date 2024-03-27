#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

// Shader source code for the compute shader
const char* computeShaderSource = R"(
#version 430

layout (local_size_x = 16, local_size_y = 16) in;

uniform float time;
layout(rgba8, binding = 0) writeonly uniform image2D destTex;

void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    vec4 pixel = vec4(sin(float(storePos.x) * 0.1 + time) * 0.5 + 0.5, cos(float(storePos.y) * 0.1 + time) * 0.5 + 0.5, 0.0, 1.0);
    imageStore(destTex, storePos, pixel);
}
)";

SDL_Renderer* renderer;

SDL_Texture* ConvertGLTextureToSDLTexture(GLuint glTexture, int width, int height) {
    // Retrieve pixel data from OpenGL texture
    GLubyte* pixels = new GLubyte[width * height * 4]; // Assuming RGBA format
    glBindTexture(GL_TEXTURE_2D, glTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    // Create SDL surface from pixel data
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, 32, width * 4, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        delete[] pixels;
        return nullptr;
    }

    // Create SDL texture from SDL surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        delete[] pixels;
        return nullptr;
    }

    // Clean up resources
    SDL_FreeSurface(surface);
    delete[] pixels;

    return texture;
}


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Compute Shader Output to Texture", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    glewInit();

    // Create an SDL_Texture to use as the output
    GLuint outputTexture;
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Create and compile the compute shader
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(computeShader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
        return -1;
    }

    // Create the program, attach the compute shader, and link it
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    // Check for linking errors
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(computeProgram, 512, NULL, infoLog);
        std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
        return -1;
    }

    // Bind the output texture to the compute shader
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Use the compute shader program
    glUseProgram(computeProgram);

    // Dispatch the compute shader
    glUniform1f(glGetUniformLocation(computeProgram, "time"), SDL_GetTicks() / 1000.0f); // Pass time to shader
    glDispatchCompute(16, 16, 1); // Dispatch 2D groups of 16x16 threads

    // Synchronize to make sure the computation is finished
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    
    SDL_Texture* texture = ConvertGLTextureToSDLTexture(outputTexture, 16, 16);
    bool quit = false;
    SDL_Event e;
    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                quit = true;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        glUniform1f(glGetUniformLocation(computeProgram, "time"), SDL_GetTicks() / 1000.0f); // Pass time to shader
        glDispatchCompute(16, 16, 1); // Dispatch 2D groups of 16x16 threads
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        texture = ConvertGLTextureToSDLTexture(outputTexture, 16, 16);

        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
    // Cleanup
    glDeleteShader(computeShader);
    glDeleteProgram(computeProgram);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
