#include "header.h"
#include "sdf_primitives.h"
#include "GameRenderer.h"
#include "textre_drawer.h"

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


    ObjectScene scene;


    LineObject arm1 = LineObject(vec3(0., 0., 0.), vec3(0., 0., 0.), vec3(0., -5., 0.), .5);
    arm1.translation_offset = vec3(0., 0., 0.);
    scene.objects.push_back(&arm1);

    /*LineObject arm2 = LineObject(vec3(5., 5., 5.), vec3(0., 0., 0.), vec3(0., -5., 0.), .5);
    arm2.translation_offset = vec3(0., 0., 0.);
    scene.objects.push_back(&arm2);*/



    /*LineObject arm11 = LineObject(vec3(0.), vec3(0.), vec3(1., -3., 0.), .5);
    arm11.translation_offset = vec3(9., -2., 0.);
    scene.objects.push_back(&arm11);*/



    /*SphereObject sphere = SphereObject(vec3(0.), 1.5);
    sphere.position = vec3(sin(3.1415 * 2. / 5. * 1.) * 5., cos(3.1415 * 2. / 5. * 1.) * 5., 0.);
    scene.objects.push_back(&sphere);

    BoxObject box = BoxObject(vec3(0.), vec3(1.5));
    box.position = vec3(sin(3.1415 * 2. / 5. * 2.) * 5., cos(3.1415 * 2. / 5. * 2.) * 5., 0.);
    scene.objects.push_back(&box);

    LineObject line = LineObject(vec3(0.), vec3(-0.9), vec3(1.), 1.5);
    line.position = vec3(sin(3.1415 * 2. / 5. * 3.) * 5., cos(3.1415 * 2. / 5. * 3.) * 5., 0.);
    scene.objects.push_back(&line);


    CylinderObject cyl = CylinderObject(vec3(0.), vec3(-1.), vec3(1.), 1.5);
    cyl.position = vec3(sin(3.1415 * 2. / 5. * 4.) * 5., cos(3.1415 * 2. / 5. * 4.) * 5., 0.);
    scene.objects.push_back(&cyl);
    CylinderObject cyl2 = CylinderObject(vec3(0., 0., 0.), vec3(-1.2), vec3(-0.5), 2.);
    cyl2.position = vec3(sin(3.1415 * 2. / 5. * 4.) * 5., cos(3.1415 * 2. / 5. * 4.) * 5., 0.);
    scene.objects.push_back(&cyl2);*/


    /*TriangleObject tri = TriangleObject(vec3(0.), vec3(0., 2.5, 0.), vec3(2.5, -2.5, 0.), vec3(-2.5, -2.5, 0.), 0.);
    tri.position = vec3(sin(3.1415 * 2. / 5. * 5.) * 5., cos(3.1415 * 2. / 5. * 5.) * 5., 0.);
    tri.rounding = 0.;
    scene.objects.push_back(&tri);*/

    int TEX_SIZE = 64;
    GLuint character_texture;
    glGenTextures(1, &character_texture);
    glBindTexture(GL_TEXTURE_3D, character_texture);
    TextureDrawer drawer = TextureDrawer(TEX_SIZE, TEX_SIZE, TEX_SIZE);
    drawer.fill(DRAWER_WHITE);

    for (int z = 0; z < TEX_SIZE; z++)
    for (int y = 0; y < TEX_SIZE; y++)
    for (int x = 0; x < TEX_SIZE; x++)
    drawer.set_pixel(x, y, z, (float)x / (float)TEX_SIZE * 255., (float)y / (float)TEX_SIZE * 255., (float)z / (float)TEX_SIZE * 255.);

    GLubyte* character_texture_data = drawer.get_data();
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB4, TEX_SIZE, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, character_texture_data);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    
    

    drawer.destroy();
    


    
    

    PrimitiveScene primitive_scene;
    scene.update_primitive_scene(&primitive_scene);


    SDF_Shader shader = SDF_Shader("assets/shader.glsl", &game_renderer.debugger);
    shader.init(TARGET_WIDTH, TARGET_HEIGHT);



    shader.set_scene(&primitive_scene);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, character_texture);
    glUniform1i(glGetUniformLocation(shader.computeProgram, "character_texture"), 0);
    
    float anglex = 0.;
    float angley = 0.;
    float anglez = 0.;

    SDL_Event e;
    while (game_renderer.is_running()) {
        float time = SDL_GetTicks() / 1000.0f;
        angley = (float)((int)(time*1000) % 31415) / 1000.0f;
        anglex = cos(time);
        anglez = sin(time);
        mat3x3 rotmatx = mat3x3{
                1, 0, 0,
                0, cos(anglex), -sin(anglex),
                0, sin(anglex), cos(anglex)
        };
        mat3x3 rotmaty = mat3x3{
                cos(angley), 0, sin(angley),
                0, 1, 0,
                -sin(angley), 0, cos(angley)
        };
        mat3x3 rotmatz = mat3x3{
                cos(anglez), -sin(anglez), 0,
                sin(anglez), cos(anglez), 0,
                0, 0, 1
        };
        mat3x3 rotmat = rotmatx * rotmaty * rotmatz;
        //sphere.transform = rotmat;
        //box.transform = rotmat;
        for (auto obj : scene.objects){
            obj->transform = rotmat;
            //obj->position = normalize(obj->position) * (float)(sin(time) * 0.5 + 0.6) * 5.5f;
        }
        
        
        shader.check_file_updates();


        shader.use();
        

        while (SDL_PollEvent(&e) != 0) {
            game_renderer.handle_event(e);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        game_renderer.switch_to_main();
        
        shader.set_1f("time", time);
        shader.set_2f("center", TARGET_WIDTH / 2., TARGET_HEIGHT / 2.);

        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_3D, character_texture);
        //glUniform1i(glGetUniformLocation(shader.computeProgram, "character_texture"), 0);
        
        scene.update_primitive_scene(&primitive_scene);
        shader.set_scene(&primitive_scene);
        

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
