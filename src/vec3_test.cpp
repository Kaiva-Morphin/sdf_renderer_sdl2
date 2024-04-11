#include "header.h"
#include "GameRenderer.h"


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>



GameRenderer game_renderer = GameRenderer();
int main(int argc, char* argv[]) {
    game_renderer.init();
    game_renderer.debugger.register_basic();
    char char_buf[128];
    snprintf(char_buf, sizeof(char_buf), "%ix%i", TARGET_WIDTH, TARGET_HEIGHT);
    game_renderer.debugger.register_line(string("resolution"), string("Resolution: "), string(char_buf));
    struct alignas(16) Primitive {
        mat4x4 transform;
        vec4 texture_position;
        vec4 position; 
        int primitive_type;
        //vec3 translation_offset;
        float rounding;
        //vec3 a;
        //vec3 b;
        //vec3 c;
    };
    Primitive primitives[16];
    primitives[0] = Primitive {
        mat4x4(31.),
        {2., 3., 4., 5.},
        vec4(3.),
        1,
        123.,
        //vec3(4.),
        //mat3x3(5.),
        //6.,
        //vec3(7.),
        //vec3(8.),
        //vec3(9.),
    };
    primitives[1] = primitives[0];
    primitives[14] = Primitive {
        mat4x4(132.),
        {123., 124., 125., 126.},
        vec4(13.),
        11,
        555.,

        //vec3(14.),
        //mat3x3(15.),
        //16.,
        //vec3(17.),
        //vec3(18.),
        //vec3(19.),
    };
    

    SDF_Shader shader = SDF_Shader("assets/test.glsl", &game_renderer.debugger);
    shader.init(TARGET_WIDTH, TARGET_HEIGHT);

    GLuint bufferID;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Primitive) * 16, primitives, GL_STATIC_DRAW);


    
    float anglex = 0.;
    float angley = 0.;
    float anglez = 0.;

    SDL_Event e;
    while (game_renderer.is_running()) {
        shader.check_file_updates();
        shader.use();
        while (SDL_PollEvent(&e) != 0) {
            game_renderer.handle_event(e);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        game_renderer.switch_to_main();
        
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Primitive) * 16, primitives, GL_STATIC_DRAW);
        shader.run();
        shader.wait();

        
        SDL_RenderCopy(renderer, shader.get_texture(), nullptr, nullptr);

        game_renderer.debugger.update_basic();
        game_renderer.debugger.draw();
        game_renderer.appy_main();
        
        SDL_RenderPresent(renderer);
    }

    //glDeleteTextures(1, &character_texture);
    
    shader.destroy();
    game_renderer.destroy();

    return 0;
}
