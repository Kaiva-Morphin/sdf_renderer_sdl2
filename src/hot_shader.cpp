#include "header.h"
#include "GameRenderer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Shader source code for the compute shader
const char* computeShaderSource = R"(
#version 430

layout (local_size_x = 16, local_size_y = 16) in;

uniform float time;
uniform vec2 center;
layout(rgba8, binding = 0) writeonly uniform image2D destTex;

void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

    float dist = (clamp(length(storePos.xy - center), 40., 60.) - 40.) / 20.;

    vec4 pixel = vec4(sin(float(storePos.x) * 0.1 + time) * 0.5 + 0.5, cos(float(storePos.y) * 0.1 + time) * 0.5 + 0.5, dist, 1.0);

    imageStore(destTex, storePos, pixel);
}
)";


std::string readShaderSource(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

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
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);  // todo: update instead of recreate
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


GameRenderer game_renderer = GameRenderer();
int main(int argc, char* argv[]) {
    game_renderer.init();
    game_renderer.debugger.register_basic();
    SDF_Shader shader = SDF_Shader("assets/shader.glsl", &game_renderer.debugger);
    shader.init(TARGET_WIDTH, TARGET_HEIGHT);
    shader.use();
    glUniform1f(glGetUniformLocation(shader.computeProgram, "time"), SDL_GetTicks() / 1000.0f);
    glUniform2f(glGetUniformLocation(shader.computeProgram, "center"), TARGET_WIDTH / 2., TARGET_HEIGHT / 2.);
    shader.run();
    shader.wait();
    
    SDL_Texture* texture = shader.get_texture(); //ConvertGLTextureToSDLTexture(outputTexture, TARGET_WIDTH, TARGET_HEIGHT);

    SDL_Event e;
    while (game_renderer.is_running()) {
        shader.check_file_updates();
        while (SDL_PollEvent(&e) != 0) {
            game_renderer.handle_event(e);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        game_renderer.switch_to_main();

        glUniform1f(glGetUniformLocation(shader.computeProgram, "time"), SDL_GetTicks() / 1000.0f); // Pass time to shader
        glUniform2f(glGetUniformLocation(shader.computeProgram, "center"), TARGET_WIDTH / 2., TARGET_HEIGHT / 2.);

        glDispatchCompute(TARGET_WIDTH / 16, TARGET_HEIGHT / 16, 1); // Dispatch 2D groups of 16x16 threads
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        texture = shader.get_texture();

        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        game_renderer.debugger.update_basic();
        game_renderer.debugger.draw();
        game_renderer.appy_main();
        
        SDL_RenderPresent(renderer);
    }
    shader.destroy();
    game_renderer.destroy();

    return 0;
}
