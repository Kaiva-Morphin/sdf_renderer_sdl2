#include "header.h"
#include "GameRenderer.h"
#include "sdf_primitives.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>



GameRenderer game_renderer = GameRenderer();
int main(int argc, char* argv[]) {
    game_renderer.init();
    game_renderer.debugger.register_basic();
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%ix%i", TARGET_WIDTH, TARGET_HEIGHT);
    game_renderer.debugger.register_line(string("resolution"), string("Resolution: "), string(buffer));


    ObjectScene scene;
    BoxObject box = BoxObject(vec3(0.), vec3(5.));
    box.position = vec3(0.);
    scene.objects.push_back(&box);
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
    scene.objects.push_back(&cyl2);


    TriangleObject tri = TriangleObject(vec3(0.), vec3(0., 2.5, 0.), vec3(2.5, -2.5, 0.), vec3(-2.5, -2.5, 0.), 0.);
    tri.position = vec3(sin(3.1415 * 2. / 5. * 5.) * 5., cos(3.1415 * 2. / 5. * 5.) * 5., 0.);
    tri.rounding = 0.;
    scene.objects.push_back(&tri);*/
    PrimitiveScene primitive_scene;
    scene.update_primitive_scene(&primitive_scene);

    int TEX_SIZE = 32;
    const int TEX_AMOUNT = 32;
    //GLubyte* data = new GLubyte[TEX_SIZE * 32 * TEX_SIZE * 3];
    GLubyte* data = new GLubyte[TEX_SIZE * TEX_SIZE * TEX_SIZE * 3];
    for (int z = 0; z < TEX_SIZE; ++z) {
        for (int y = 0; y < TEX_SIZE; ++y) {
            for (int x = 0; x < TEX_SIZE; ++x) {
                int index = (z * TEX_SIZE * TEX_SIZE + y * TEX_SIZE + x) * 3;
                data[index] = (int)((float)x / (float)TEX_SIZE * 255.);
                data[index + 1] = (float)y / (float)TEX_SIZE * 255.;
                data[index + 2] = (float)z / (float)TEX_SIZE * 255.;
                //data[(z * TEX_SIZE * TEX_SIZE + y * TEX_SIZE + x) * 3] = static_cast<GLubyte>(x/TEX_SIZE*255); // Red
                //data[(z * TEX_SIZE * TEX_SIZE + y * TEX_SIZE + x) * 3 + 1] = static_cast<GLubyte>(y/TEX_SIZE*255); // Green
                //data[(z * TEX_SIZE * TEX_SIZE + y * TEX_SIZE + x) * 3 + 2] = static_cast<GLubyte>(z/TEX_SIZE*255); // Blue

            }
        }
    }

    


    GLuint textures[TEX_AMOUNT];
    for (int i = 0; i < TEX_AMOUNT; ++i) {
        glGenTextures(TEX_AMOUNT, &textures[i]);
        glBindTexture(GL_TEXTURE_3D, textures[i]);
        GLubyte* texdata = new GLubyte[TEX_SIZE * TEX_SIZE * TEX_SIZE * 3];
        if (i == 0) std::fill(texdata, texdata + TEX_SIZE * TEX_SIZE * TEX_SIZE * 3, 255.);
        if (i == 1) std::fill(texdata, texdata + TEX_SIZE * TEX_SIZE * TEX_SIZE * 3, 128.);
        if (i == 2) std::fill(texdata, texdata + TEX_SIZE * TEX_SIZE * TEX_SIZE * 3, 0.);
        if (i == 3) std::fill(texdata, texdata + TEX_SIZE * TEX_SIZE * TEX_SIZE * 3, 255.);
        if (i == 4) std::fill(texdata, texdata + TEX_SIZE * TEX_SIZE * TEX_SIZE * 3, 255.);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB4, TEX_SIZE, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, texdata);
        //glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB4, TEX_SIZE, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, i==0?data:nullptr);
        //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    }

    delete[] data;

    snprintf(buffer, sizeof(buffer), "%i bytes", sizeof(primitive_scene));
    game_renderer.debugger.register_line(string("s_usage"), string("Struct usage: "), string(buffer));
    snprintf(buffer, sizeof(buffer), "%f Mbytes", (double)(TEX_SIZE * TEX_SIZE * TEX_SIZE * 12 * TEX_AMOUNT) / 8. / 1024. / 1024.);
    game_renderer.debugger.register_line(string("t_usage"), string("Texture usage: "), string(buffer));


    SDF_Shader shader = SDF_Shader("assets/shader.glsl", &game_renderer.debugger);
    shader.init(TARGET_WIDTH, TARGET_HEIGHT);
    shader.use();
    glUniform1f(glGetUniformLocation(shader.computeProgram, "time"), SDL_GetTicks() / 1000.0f);
    glUniform2f(glGetUniformLocation(shader.computeProgram, "center"), TARGET_WIDTH / 2., TARGET_HEIGHT / 2.);
    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, textures[0]);
    glUniform1i(glGetUniformLocation(shader.computeProgram, "textures[2]"), 0);
    glActiveTexture(GL_TEXTURE);

    glBindTexture(GL_TEXTURE_3D, textures[1]);
    glUniform1i(glGetUniformLocation(shader.computeProgram, "textures[1]"), 1);*/
    for (int i = 0; i < TEX_AMOUNT; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_3D, textures[i]);
        glUniform1i(glGetUniformLocation(shader.computeProgram, ("textures[" + std::to_string(i) + "]").c_str()), i);
    }

    

    GLuint scenebuffer;
    glGenBuffers(1, &scenebuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, scenebuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(primitive_scene), &primitive_scene, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scenebuffer);

    //shader.run();
    //shader.wait();

    float anglex = 0.;
    float angley = 0.;
    float anglez = 0.;

    SDL_Texture* texture;// = shader.get_texture(); //ConvertGLTextureToSDLTexture(outputTexture, TARGET_WIDTH, TARGET_HEIGHT);
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
        cout << scene.objects.size() << endl;
        for (Object* obj :scene.objects){
            obj->transform = rotmat;
        }
            //
            //obj->position = vec3(0.);
            //obj->position = 5.0f * (float)(sin(time) * 0.5 + 0.6) * normalize(obj->position) ;
        
        //sphere.position.x = sin(time) * 1.;
        //sphere.position.y = cos(time) * 1.;

        scene.update_primitive_scene(&primitive_scene);
        shader.check_file_updates();
        while (SDL_PollEvent(&e) != 0) {
            game_renderer.handle_event(e);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        game_renderer.switch_to_main();

        glUniform1f(glGetUniformLocation(shader.computeProgram, "time"), time); // Pass time to shader
        glUniform2f(glGetUniformLocation(shader.computeProgram, "center"), TARGET_WIDTH / 2., TARGET_HEIGHT / 2.);

        /*for (int i = 0; i < TEX_AMOUNT; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_3D, textures[i]);
            glUniform1i(glGetUniformLocation(shader.computeProgram, ("textures[" + std::to_string(i) + "]").c_str()), i);
        }*/

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, scenebuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(primitive_scene), &primitive_scene, GL_STATIC_DRAW);
        glDispatchCompute(TARGET_WIDTH / 16, TARGET_HEIGHT / 16, 1); // Dispatch 2D groups of 16x16 threads
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        texture = shader.get_texture();

        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        game_renderer.debugger.update_basic();
        game_renderer.debugger.draw();
        game_renderer.appy_main();
        
        SDL_RenderPresent(renderer);
    }
    for (int i = 0; i < TEX_AMOUNT; ++i) {
        glDeleteTextures(1, &textures[i]); // Delete each texture
    }
    glDeleteBuffers(1, &scenebuffer);
    shader.destroy();
    game_renderer.destroy();

    return 0;
}
