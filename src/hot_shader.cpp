#include "header.h"
#include "GameRenderer.h"
#include "sdf_primitives.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


GameRenderer game_renderer = GameRenderer();
int main(int argc, char* argv[]) {
    game_renderer.init();
    game_renderer.debugger.register_basic();

    PrimitiveScene scene;
    scene.size = 3;
    scene.primitives[0] = Primitive{
        2, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
        // universal
        vec3(2),
        vec3(2),
        mat3x3(2., 2., 2., 2., 2., 2., 2., 2., 2.),
        2.,
        // specific points
        vec3(2),
        vec3(2),
        vec3(2),
    };
    SDF_Shader shader = SDF_Shader("assets/shader.glsl", &game_renderer.debugger);
    shader.init(TARGET_WIDTH, TARGET_HEIGHT);
    shader.use();
    glUniform1f(glGetUniformLocation(shader.computeProgram, "time"), SDL_GetTicks() / 1000.0f);
    glUniform2f(glGetUniformLocation(shader.computeProgram, "center"), TARGET_WIDTH / 2., TARGET_HEIGHT / 2.);
    
    GLuint scenebuffer;
    glGenBuffers(1, &scenebuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scenebuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(scene), &scene, GL_DYNAMIC_DRAW);

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
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(scene), &scene, GL_DYNAMIC_DRAW);
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
