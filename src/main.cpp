#include <SDL2/SDL.h>
#include "cmath"


#include <glm/glm.hpp>
using namespace glm;

#include "graphics.h"
#include "sdf.h"

#include <iostream>
#include <variant>
using namespace std;

#include <chrono>
#include <ctime> 

int main(int argc, char ** argv)
{
    
    Scene scene;
    //SphereObject obj1 = SphereObject{vec3{62, 0, 0 + 120 + 25}, 16};
    //scene.objects.push_back(&obj1);
    BoxObject obj2 = BoxObject{vec3{82, 10, 60}, vec3{16, 16, 16}};
    scene.objects.push_back(&obj2);
    //SphereObject obj3 = SphereObject{vec3{82, 0, 0 + 120 + 25}, 16};
    //scene.objects.push_back(&obj3);
    //SphereObject obj4 = SphereObject{vec3{52, 0, 0 + 60 + 25}, 16};
    //scene.objects.push_back(&obj4);
    
    init();
    bool quit = false;

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 80, 80);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);


    vec3 sun_vec = vec3{1, -3, 1};
    float anglex = 0.;
    
    
    if(false){
        while (!check_quit_event())
        {

            mat3x3 rotmat = mat3x3{
                cos(anglex), 0, sin(anglex),
                0, 1, 0,
                -sin(anglex), 0, cos(anglex)
            };
            vec3 sun_vector = sun_vec;

            //rotmat

            anglex += 0.5;
            SDL_Delay(10);
            obj2.rotation = rotmat;

            auto start = std::chrono::system_clock::now();


            clear_screen(100, 100, 255); // draw sky

            scene.objects[0]->rotation = rotmat;
            


            
            for (int x = 0; x < 120; ++x){
                for (int y = 0; y < 120; ++y){
                    vec3 pos = vec3{x + 30, y - 30, 1};
                    vec3 direction = vec3{0, 0, 1}; // orthoganal camera
                    vec3 hit_point;
                    double result;
                    tie(hit_point, result) = RaySceneSDF(pos, direction, &scene);
                    if (result < SDF_INF){
                        vec3 normal = CalculateNormal(hit_point, &scene);
                        double dot_product = 0.1 + 0.9 * (dot(normalize(sun_vector), normal) * 0.5 + 0.5);
                        double amp = clamp(dot_product, 0., 1.);
                        double c = round((amp * 255.));
                        SDL_SetRenderDrawColor(renderer, c, c, c, 255);
                        SDL_RenderDrawPoint(renderer, x, y);
                    } else {
                        //SDL_RenderDrawPoint(renderer, x, y);
                    }
                }
            }

            draw();
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            //cout <<  1. / elapsed_seconds.count() << "\n";
        }
    } else {
        while (!check_quit_event())
        {
            
            mat3x3 rotmat = mat3x3{
                cos(anglex), 0, sin(anglex),
                0, 1, 0,
                -sin(anglex), 0, cos(anglex)
            };
            vec3 sun_vector = sun_vec * rotmat;
            anglex += 0.1;
            SDL_Delay(10);


            obj2.rotation = rotmat;

            auto start = std::chrono::system_clock::now();

            clear_screen(100, 100, 255); // draw sky
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            DrawSceneTD(&scene);
            
            for (int x = 10; x < 80; x+=2){
                vec3 pos = vec3{x + 30, 26, 1};
                vec3 direction = vec3{0, 0, 1}; // orthoganal camera

                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawPoint(renderer, pos.z, pos.x);
                
                vec3 hit_pos;
                double hit_dist;
                tie(hit_pos, hit_dist) = RaySceneSDF(pos, direction, &scene, true);
        
                /*vec3 hit_point;
                double result;
                double min_dist = 0.1;
                double max_dist = 100.;
                direction = normalize(direction);
                vec3 position = pos;
                int steps = 64;
                bool is_hit = false;
                for (int step = 0; step < steps; ++step){
                    double dist = SampleSceneSDF(position, &scene);
                    if (dist <= min_dist) {
                        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                        SDL_RenderDrawPoint(renderer, position.z, position.x);
                        is_hit = true;
                        break;
                    };
                    if (dist >= max_dist) {break;};
                    vec3 step_vec = direction;
                    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 90);
                    DrawCircle(position.z, position.x, dist);
                    step_vec *= dist;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 90);
                    SDL_RenderDrawLine(renderer, 
                        (position).z, (position).x,
                        (position + step_vec).z, (position + step_vec).x
                    );
                    position += step_vec;
                }
                if (!is_hit){
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    SDL_RenderDrawPoint(renderer, position.z, position.x);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                    SDL_RenderDrawPoint(renderer, pos.z, pos.x);
                }*/
                
            }

            draw();
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            //cout <<  1. / elapsed_seconds.count() << "\n";
        }
    }
    

    SDL_DestroyTexture(texture);

    cleanup();
    return 0;
}