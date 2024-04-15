#include "header.h"
#include "sdf_primitives.h"
#include "Game.h"
#include "textre_drawer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

mat4x4 eulerXYZ(float anglex, float angley, float anglez){
    anglex = anglex / 180. * 3.1415;
    angley = angley / 180. * 3.1415;
    anglez = anglez / 180. * 3.1415;
    mat4x4 rotmatx = {
            1, 0., 0., 0.,
            0., cos(anglex), -sin(anglex), 0.,
            0., sin(anglex), cos(anglex), 0.,
            0., 0., 0., 1,
    };
    mat4x4 rotmaty = {
            cos(angley), 0., sin(angley), 0.,
            0., 1, 0., 0.,
            -sin(angley), 0., cos(angley), 0.,
            0., 0., 0., 1,
    };
    mat4x4 rotmatz = {
            cos(anglez), -sin(anglez), 0., 0.,
            sin(anglez), cos(anglez), 0., 0.,
            0., 0., 1, 0.,
            0., 0., 0., 1,
    };
    return rotmatx * rotmaty * rotmatz;
}

Game game = Game();
int main(int argc, char* argv[]) {
    game.init();
    game.debugger.register_basic();

    ObjectScene scene;
    for (int i = 0; i < 6; i ++){
        BoxObject* box = new BoxObject(vec3(0., 0., 0.), vec3(1., 1., 1.));
        switch (i){
            case 0:
            box->set_translation_offset({3., 0., 0.,});
            box->texture_transform = mat4x4(
                1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                30., 30., -30, 1.
            );
            break;
            case 1:
            box->set_translation_offset({0., 3., 0.,});
            box->texture_transform = mat4x4(
                1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                30., -30., 30, 1.
            );
            break;
            case 2:
            box->set_translation_offset({0., 0., 3.,});
            box->texture_transform = mat4x4(
                1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                30., -30., -30, 1.
            );
            break;
            case 3:
            box->set_translation_offset({-3., 0., 0.,});
            box->texture_transform = mat4x4(
                1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                -30., 30., 30, 1.
            );
            break;
            case 4:
            box->set_translation_offset({0., -3., 0.,});
            box->texture_transform = mat4x4(
                1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                -30., 30., -30, 1.
            );
            break;
            case 5:
            box->set_translation_offset({0., 0., -3.,});
            box->texture_transform = mat4x4(
                1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                -30., -30., 30, 1.
            );
            break;
        }
        scene.objects.push_back(box);
        scene.ordered_operations.push_back(
            PrimitiveOperation{
                OPERATION_UNION,
                i-1,
                i,
                i,
                3
            }
        );
    }
    int sphere_id = (int)scene.ordered_operations.size();
    SphereObject sphere = SphereObject(vec3(0.), 2.5);
    sphere.texture_transform = mat4x4(
                1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                -30., -30., -30, 1.
    );
    scene.objects.push_back(&sphere);
    scene.ordered_operations.push_back(
        PrimitiveOperation{
                OPERATION_INTERPOLATION,
                (int)scene.ordered_operations.size(),
                (int)scene.ordered_operations.size()-1,
                (int)scene.ordered_operations.size(),
                1.
        }
    );
    int TEX_SIZE = 64;
    GLuint character_texture;
    glGenTextures(1, &character_texture);
    glBindTexture(GL_TEXTURE_3D, character_texture);
    TextureDrawer drawer = TextureDrawer(TEX_SIZE, TEX_SIZE, TEX_SIZE);
    drawer.fill(DRAWER_WHITE);
    for (int z = 0; z < TEX_SIZE; z++)
    for (int y = 0; y < TEX_SIZE; y++)
    for (int x = 0; x < TEX_SIZE; x++)
    drawer.set_pixel(x, y, z, x > TEX_SIZE * 0.5 ? 0. : 255., y > TEX_SIZE * 0.5 ? 0. : 255., z > TEX_SIZE * 0.5 ? 0. : 255.);
    GLubyte* character_texture_data = drawer.get_data();
    PrimitiveScene primitive_scene;
    scene.update_primitive_scene(&primitive_scene);
    SDF_Shader shader = SDF_Shader("assets/shader.glsl", &game.debugger);
    shader.init(TARGET_WIDTH, TARGET_HEIGHT, ivec3(TEX_SIZE), character_texture_data);
    drawer.destroy();
    shader.set_scene(&primitive_scene);
    float anglex = 0.;
    float angley = 0.;
    float anglez = 0.;
    SDL_Event e;
    while (game.is_running()) {
        float time = SDL_GetTicks() / 1000.0f;
        angley = (float)((int)(time*1000) % 31415) / 1000.0f;
        anglex = cos(time) * 3.;
        anglez = sin(time * 0.5) * 3.;
        mat4x4 rotmatx = {
                1, 0, 0, 0,
                0, cos(anglex), -sin(anglex), 0,
                0, sin(anglex), cos(anglex), 0,
                0, 0, 0, 0,
        };
        mat4x4 rotmaty = {
                cos(angley), 0, sin(angley), 0,
                0, 1, 0, 0,
                -sin(angley), 0, cos(angley), 0,
                0, 0, 0, 0,
        };
        mat4x4 rotmatz = {
                cos(anglez), -sin(anglez), 0, 0,
                sin(anglez), cos(anglez), 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 0,
        };
        mat4x4 rotmat = rotmatx * rotmaty * rotmatz;
        //sphere.transform = rotmat;
        //box.transform = rotmat;
        int c = 0;
        for (auto obj : scene.objects){
            obj->transform[0] = rotmat[c%3];
            obj->transform[1] = rotmat[(c+1)%3];
            obj->transform[2] = rotmat[(c+2)%3];
            //c++;
            //obj->transform[3] = vec4(obj->translation_offset, 1.);
            //obj->position = normalize(obj->position) * (float)(sin(time) * 0.5 + 0.6) * 5.5f;
        }
        shader.check_file_updates();
        shader.use();
        while (SDL_PollEvent(&e) != 0) {
            game.handle_event(e);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        game.switch_to_main();
        shader.set_1f("time", time);
        shader.set_2f("center", TARGET_WIDTH / 2., TARGET_HEIGHT / 2.);
        scene.update_primitive_scene(&primitive_scene);
        shader.set_scene(&primitive_scene);
        shader.run();
        shader.wait();
        
        SDL_RenderCopy(renderer, shader.get_texture(), nullptr, nullptr);

        game.debugger.update_basic();
        game.debugger.draw();
        game.apply_main();
        
        SDL_RenderPresent(renderer);
    }

    //glDeleteTextures(1, &character_texture);
    
    shader.destroy();
    game.destroy();

    return 0;
}
