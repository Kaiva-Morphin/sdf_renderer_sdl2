#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <gl/GLU.h>
#include "cmath"
#include <chrono>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

const int TARGET_WIDTH = 240;
const int TARGET_HEIGHT = 144;
const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;

double get_median(vector<double> vec){
    double summ = 0;
    for (int i = 0; i < vec.size(); i++){
        summ += vec[i];
    }
    return summ / vec.size();
}

int main(int argc, char ** argv){
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Simple Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
    SDL_SetWindowResizable(window, SDL_bool::SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, TARGET_WIDTH, TARGET_HEIGHT);

    vector<double> times;
    for (int i = 0; i < 10000; i++){
        auto start = std::chrono::system_clock::now();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        times.push_back(1. / elapsed_seconds.count());
    }
    cout << "Only fill: " << get_median(times) << endl;
    times.clear();

    /*for (int i = 0; i < 1000; i++){
        auto start = std::chrono::system_clock::now();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
        SDL_RenderFillRect(renderer, &rect);
        for (int x = 0; x < TARGET_WIDTH; ++x){
            for (int y = 0; y < TARGET_HEIGHT; ++y){
                SDL_SetRenderDrawColor(renderer, round((double)x / (double)TARGET_WIDTH * 255.), round((double)y / (double)TARGET_HEIGHT * 255.), 0, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
        SDL_RenderPresent(renderer);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        times.push_back(1. / elapsed_seconds.count());
    }
    cout << "Single pixel: " << get_median(times) << endl;
    times.clear();/**/


    SDL_Texture *buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, TARGET_WIDTH, TARGET_HEIGHT);
    for (int i = 0; i < 10000; i++){
        auto start = std::chrono::system_clock::now();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
        SDL_RenderFillRect(renderer, &rect);
        int *pixels = NULL;
        int pitch;
        SDL_LockTexture(buffer, &rect, (void **) &pixels, &pitch);
        //#pragma omp parallel for
        for (int x = 0; x < TARGET_WIDTH; ++x){
            for (int y = 0; y < TARGET_HEIGHT; ++y){
                int r,g,b,a;
                r = round((double)x / (double)TARGET_WIDTH * 255.);
                g = round((double)y / (double)TARGET_HEIGHT * 255.);
                b = 0;
                a = 255;
                pixels[x + y * TARGET_WIDTH] = (static_cast<uint32_t>(r) << 24) |
                                                (static_cast<uint32_t>(g) << 16) |
                                                (static_cast<uint32_t>(b) << 8) |
                                                static_cast<uint32_t>(a);
            }
        }
        SDL_UnlockTexture(buffer);
        SDL_RenderCopy(renderer, buffer, NULL, NULL);
        SDL_RenderPresent(renderer);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        times.push_back(1. / elapsed_seconds.count());
    }
    cout << "Direct to texture: " << get_median(times) << endl;
    times.clear();

    for (int i = 0; i < 10000; i++){
        auto start = std::chrono::system_clock::now();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
        SDL_RenderFillRect(renderer, &rect);
        int *pixels = NULL;
        int pitch;
        SDL_LockTexture(buffer, &rect, (void **) &pixels, &pitch);
        #pragma omp parallel for
        for (int x = 0; x < TARGET_WIDTH; ++x){
            for (int y = 0; y < TARGET_HEIGHT; ++y){
                int r,g,b,a;
                r = round((double)x / (double)TARGET_WIDTH * 255.);
                g = round((double)y / (double)TARGET_HEIGHT * 255.);
                b = 0;
                a = 255;
                pixels[x + y * TARGET_WIDTH] = (static_cast<uint32_t>(r) << 24) |
                                                (static_cast<uint32_t>(g) << 16) |
                                                (static_cast<uint32_t>(b) << 8) |
                                                static_cast<uint32_t>(a);
            }
        }
        SDL_UnlockTexture(buffer);
        SDL_RenderCopy(renderer, buffer, NULL, NULL);
        SDL_RenderPresent(renderer);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        times.push_back(1. / elapsed_seconds.count());
    }
    cout << "Multithreaded direct to texture: " << get_median(times) << endl;
    times.clear();


    SDL_DestroyTexture(buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}