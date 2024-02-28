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

int main(int argc, char ** argv)
{
    
    Scene scene;
    /*
    LineObject obj1 = LineObject{vec3{82, -30, 90}, vec3{0, -10, 0}, vec3{0, -10, 0}, 9, eye3};
    scene.objects.push_back(&obj1);/**/
    ///*
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
        
        
        angley += 0.1;
        anglez += 0.1;
        //SDL_Delay(10);
        vec3 sun_vector = sun_vec * rotmaty;
        obj4.transform = rotmat;
        obj1.translation_offset = vec3{0, sin(PI / 4. + angley * 3.) * 20, 0};
        obj2.translation_offset = vec3{0, sin(PI / 4. * 2. + angley * 3.) * 20, 0};
        obj3.translation_offset = vec3{0, sin(PI / 4. * 3. + angley * 3.) * 20, 0};
        obj4.translation_offset = vec3{0, sin(PI / 4. * 4. + angley * 3.) * 20, 0};


        
        
        if(true){
            auto start = std::chrono::system_clock::now();
            clear_screen(100, 100, 255); // draw sky

            int *pixels = NULL;
            int pitch;
            SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
            SDL_LockTexture(buffer, &rect, (void **) &pixels, &pitch);
            
            vector<thread> threads;
            uint32_t results[TARGET_WIDTH][TARGET_HEIGHT];
            for (int x = 0; x < TARGET_WIDTH; ++x){
                //for (int y = 0; y < TARGET_HEIGHT; ++y){
                threads.push_back(thread(compute_row, x, &scene, sun_vector, results[x]));
                    //threads.push_back(thread(threaded_pixel, x, y, &scene, sun_vector, &results[x]));
                    //compute_row(x, &scene, sun_vector, &results[x]);
                //}
            }
            for (int x = 0; x < threads.size(); ++x){
                threads[x].join();
                for (int y = 0; y < TARGET_HEIGHT; ++y){
                    pixels[x + y * TARGET_WIDTH] = results[x][y];
                };
            }
            SDL_UnlockTexture(buffer);
            SDL_RenderCopy(renderer, buffer, NULL, NULL);
            draw();
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            cout <<  1. / elapsed_seconds.count() << "\n";
        } else {
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
                tie(hit_pos, hit_dist) = RaySceneSDF(pos, direction, &scene, true);
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