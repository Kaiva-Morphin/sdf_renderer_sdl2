#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int LOGICAL_HEIGHT = 240; // Set the logical height for your camera

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

bool init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        //printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // Create window
    gWindow = SDL_CreateWindow("Low Resolution Camera",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               SCREEN_WIDTH,
                               SCREEN_HEIGHT,
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (gWindow == NULL) {
        //printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // Create renderer
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
        //printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // Set logical size
    SDL_RenderSetLogicalSize(gRenderer, SCREEN_WIDTH, LOGICAL_HEIGHT);

    return true;
}

void close() {
    // Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}

void render() {
    // Clear screen
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(gRenderer);

    // Render your graphics here

    // Update screen
    SDL_RenderPresent(gRenderer);
}

int main(int argc, char* args[]) {
    // Initialize SDL
    if (!init()) {
        //printf("Failed to initialize!\n");
        return -1;
    }

    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    // Main loop
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Window event
            else if (e.type == SDL_WINDOWEVENT) {
                // Handle window resize event
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    // Set logical size based on new window size
                    SDL_RenderSetLogicalSize(gRenderer, SCREEN_WIDTH, LOGICAL_HEIGHT);
                }
            }
        }

        // Render scene
        render();
    }

    // Free resources and close SDL
    close();

    return 0;
}
