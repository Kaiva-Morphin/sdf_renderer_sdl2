#include "header.h"

#include "GameRenderer.h"





int main(int argc, char* args[]) {
    GameRenderer game_renderer = GameRenderer();
    game_renderer.init();
    game_renderer.debugger.register_basic();
    SDL_Event e;
    while (game_renderer.is_running()) {
        while (SDL_PollEvent(&e) != 0) {
            game_renderer.handle_event(e);
        }
        //SDL_RenderSetViewport(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 10, 10, 210, 255);

        SDL_RenderClear(renderer);
        //SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
        //SDL_RenderFillRect(renderer, &rect);
        //SDL_SetRenderDrawColor(renderer, 240, 240, 210, 255);

        game_renderer.switch_to_main();
        int xo = 0;
        int yo = 0;
        for (int x = xo; x < TARGET_WIDTH + xo; x++){
            for (int y = yo; y < TARGET_HEIGHT + yo; y++){
                int ax = round((float)x / (float)TARGET_WIDTH * 255.);
                int ay = round((float)y / (float)TARGET_HEIGHT * 255.);
                SDL_SetRenderDrawColor(renderer, ax, ay, (x+y)%2==0?0:255, 255);
                SDL_Rect rect = SDL_Rect{x, y, 1, 1};
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        game_renderer.debugger.update_basic();
        game_renderer.debugger.draw();
        game_renderer.appy_main();
        SDL_RenderPresent(renderer);
    }

    game_renderer.destroy();
    return 0;
}
