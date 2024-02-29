#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <gl/GLU.h>
#include "cmath"

#include <glm/glm.hpp>
using namespace glm; // gl math for vec and mat operations

#include "graphics.h"
#include "sdf.h"

#include <iostream>
#include <variant>
using namespace std;

#include <chrono>
#include <ctime>

#include <thread>

#define PI 3.14159
#define HALF_PI PI / 2.

#define red  vec4{255, 0, 0, 255}

int main(int argc, char ** argv)
{
    
    Scene scene;
    ///*

    LineObject l = LineObject{vec3{105, 70, 60}, vec3{-33, -60, 0}, vec3{-23, -30, 0}, 8, eye3};
    scene.objects.push_back(&l);
    l.color = vec4{0.8, 0.8, 0., 1.};
    
    LineObject body = LineObject{vec3{105, 70, 60}, vec3{-13, -10, 0}, vec3{17, -10, 0}, 17, eye3};
    scene.objects.push_back(&body);
    body.color = vec4{1, 0.8, 0., 1.};
    LineObject tail = LineObject{vec3{105, 70, 60}, vec3{35, -15, 0}, vec3{45, -10, 0}, 2, eye3};
    scene.objects.push_back(&tail);
    tail.color = vec4{0.4, 0.6, 0.2, 1.};

    LineObject head = LineObject{vec3{105, 70, 60}, vec3{-43, -70, 0}, vec3{-53, -63, 0}, 3, eye3};
    scene.objects.push_back(&head);
    head.color = vec4{0.4, 0.4, 0, 1.};
    LineObject leg1 = LineObject{vec3{105, 70, 60}, vec3{-20, 10, -10}, vec3{-20, 22, -14}, 4, eye3};
    scene.objects.push_back(&leg1);
    leg1.color = vec4{0.2, 0.2, 0, 1.};
    LineObject leg2 = LineObject{vec3{105, 70, 60}, vec3{-20, 10, 10}, vec3{-20, 22, 14}, 4, eye3};
    scene.objects.push_back(&leg2);
    leg2.color = vec4{0.2, 0.2, 0, 1.};
    LineObject leg3 = LineObject{vec3{105, 70, 60}, vec3{20, 10, -10}, vec3{20, 22, -14}, 4, eye3};
    scene.objects.push_back(&leg3);
    leg3.color = vec4{0.2, 0.2, 0, 1.};
    LineObject leg4 = LineObject{vec3{105, 70, 60}, vec3{20, 10, 10}, vec3{20, 22, 14}, 4, eye3};
    scene.objects.push_back(&leg4);
    leg4.color = vec4{0.2, 0.2, 0, 1.};
    
    BoxObject cube = BoxObject{vec3{105, 70, 60}, vec3{20, 20, 20}, eye3};
    scene.objects.push_back(&cube);
    cube.color = vec4{1., 1., 0, 1.};
    
    /**/
    /*
    SphereObject obj1 = SphereObject{vec3{82, 19, 60}, 10, eye3};
    scene.objects.push_back(&obj1);
    SphereObject obj2 = SphereObject{vec3{102, 19, 60}, 12, eye3};
    scene.objects.push_back(&obj2);
    SphereObject obj3 = SphereObject{vec3{122, 19, 60}, 14, eye3};
    scene.objects.push_back(&obj3);
    BoxObject obj4 = BoxObject{vec3{142, 19, 60}, vec3{16, 16, 16}, eye3};
    scene.objects.push_back(&obj4);/**/
    
    init();
    bool quit = false;

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 80, 80);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_Texture *buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, TARGET_WIDTH, TARGET_HEIGHT);
    SDL_SetTextureBlendMode(buffer, SDL_BLENDMODE_BLEND);

    vec3 sun_vec = vec3{1, -3, 1};
    float anglex = 0.;
    float angley = 0.;
    float anglez = 0.;
    while (!check_quit_event())
    {
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
        
        for (size_t object_id = 0; object_id < scene.objects.size(); object_id++) {
            scene.objects[object_id]->transform = rotmat;
            scene.objects[object_id]->translation_offset = vec3{0, -8, 40};
        }
        
        angley += 0.1;
        anglez = sin(angley);
        //SDL_Delay(10);
        vec3 sun_vector = sun_vec * rotmaty;


        /*mat3x3 rotmatx = mat3x3{
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
        mat3x3 rotmat = rotmatx * rotmaty * rotmatz;*/
        cube.translation_offset.x = sin(angley * 0.2) * 100.;








        //obj1.transform = rotmat;
        //obj1.translation_offset = vec3{0, 0, 0};
        /*obj4.transform = rotmat;
        obj1.translation_offset = vec3{0, sin(PI / 4. + angley * 3.) * 20, 0};
        obj2.translation_offset = vec3{0, sin(PI / 4. * 2. + angley * 3.) * 20, 0};
        obj3.translation_offset = vec3{0, sin(PI / 4. * 3. + angley * 3.) * 20, 0};
        obj4.translation_offset = vec3{0, sin(PI / 4. * 4. + angley * 3.) * 20, 0};*/
        if(true){
            auto start = std::chrono::system_clock::now();
            clear_screen(100, 100, 255); // draw sky

            int *pixels = NULL;
            int pitch;
            SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
            SDL_LockTexture(buffer, &rect, (void **) &pixels, &pitch);
            
            vector<thread> threads;
            const int rendersizey = TARGET_HEIGHT;
            uint32_t results[TARGET_WIDTH][rendersizey];
            for (int x = 0; x < TARGET_WIDTH; ++x){
                threads.push_back(thread(compute_row, x, rendersizey, &scene, sun_vector, results[x]));
            }
            for (int x = 0; x < threads.size(); ++x){
                threads[x].join();
                for (int y = 0; y < rendersizey; ++y){
                    pixels[x + y * TARGET_WIDTH] = results[x][y];
                };
            }
            SDL_UnlockTexture(buffer);
            SDL_RenderCopy(renderer, buffer, NULL, NULL);
            draw();
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            //cout <<  elapsed_seconds.count() << "\n";
        } else { // 2d debug renderer
            auto start = std::chrono::system_clock::now();
            clear_screen(100, 100, 255); // draw sky
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            DrawSceneTD(&scene);
            
            for (int x = 0; x < 100; x+=2){
                vec3 pos = vec3{x + 30, 10, 1};
                vec3 direction = vec3{0, 0, 1}; // orthoganal camera

                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawPoint(renderer, pos.z, pos.x);
                
                vec3 hit_pos;
                double hit_dist;
                //tie(hit_pos, hit_dist) = RaySceneSDF(pos, direction, &scene, true);
            }
            
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;

            //DrawCircle(this->position.z, this->position.x, this->radius);
            //cout <<  1. / elapsed_seconds.count() << "\n";
            draw();
        }
    }
    
    SDL_DestroyTexture(buffer);
    SDL_DestroyTexture(texture);

    cleanup();
    return 0;
}