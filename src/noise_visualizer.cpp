#include "fast_noise.h"
#include "graphics.h"
#include <SDL2/SDL.h>
#include <iostream>

using namespace std;

int main(int argc, char ** argv)
{
    init();


    FastNoiseLite noise;

    double z = 0;
    
    noise.SetSeed(3);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFrequency(0.1);
    while (!check_quit_event())
    {
        clear_screen(100, 100, 255);
        switch( event.type ){
            case SDL_KEYDOWN:
                switch( event.key.keysym.sym )
                {
                        case SDLK_UP:
                        z -= 0.3;
                        break;

                        case SDLK_DOWN:
                        z += 0.3;
                        break;

                        default:
                        break;
                }
                

                break;
            case SDL_KEYUP:
                break;

            default:
                break;
        }
        for (int x=0; x<TARGET_WIDTH;x++){
            for (int y=0;y<TARGET_HEIGHT;y++){
                double val = noise.GetNoise((float)x, (float)y, (float)z);
                val = (val + 1) * 0.5;
                if (val * 255. < 170.) {
                    SDL_SetRenderDrawColor(renderer, 150., 150., 0., 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 40., 40., 0., 255);
                }
                
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        draw();
    }



    cleanup();
    return 0;
}

