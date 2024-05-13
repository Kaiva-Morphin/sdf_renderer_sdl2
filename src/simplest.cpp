#include "Game.h"
#include "Physics.h"
#include "BdfFont.h"

SDL_Event event;

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}
float clamp(float value, float min, float max){
    return std::max(min, std::min(value, max));
}
float smoothstep(float edge0, float edge1, float x) {
    if (edge0==edge1)return edge0;
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}



struct Player{
    vec2 position = vec2(0);
    vec2 acceleration = vec2(0);
    vec2 velocity = vec2(0);
};

int main(int argc, char ** argv)
{
    Game g = Game();
    game = &g;
    game->init();
    BDFAtlas font_atlas = BDFAtlas("assets/fonts/orp/orp-book.bdf", 1536);
    game->debugger.init(&font_atlas);
    game->debugger.register_basic();
    PhysicsSolver solver("");
    bool key_a, key_d, key_w, key_s;
    key_a = false;
    key_d = false;
    key_w = false;
    key_s = false;


    Player player;

    int xMouse, yMouse;
    while (game->is_running())
    {
        SDL_Delay(1);
        Uint32 mouseState = SDL_GetMouseState(&xMouse,&yMouse);
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        vec2 target = {
            remap(xMouse, 0, w, -half_screen.x,  half_screen.x),
            remap(yMouse, 0, h,  half_screen.y, -half_screen.y)
        };

        float time = game->time();
        while (SDL_PollEvent(&event)) {
            game->handle_event(event);
            if (event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        key_a = true;
                        break;
                    case SDLK_d:
                        key_d = true;
                        break;
                    case SDLK_w:
                        player.velocity.y = 1000;
                        key_w = true;
                        break;
                    case SDLK_s:
                        key_s = true;
                        break;
                }
            }
            if (event.type == SDL_KEYUP){
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        key_a = false;
                        break;
                    case SDLK_d:
                        key_d = false;
                        break;
                    case SDLK_w:
                        key_w = false;
                        break;
                    case SDLK_s:
                        key_s = false;
                        break;
                    
                }
            }
        }
        game->begin_main();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);



        float targetx = (key_d - key_a);

        player.acceleration.x += ((targetx > 0) != (player.velocity.x > 0)) ? targetx * 10000 : targetx * 40 ;
        player.acceleration.x = clamp(player.acceleration.x, -1000, 1000);



        player.velocity.y -= 9.8;
        if (targetx == 0){player.acceleration.x = 0; player.velocity.x *= 0.999;}
        player.velocity.x += player.acceleration.x * 0.001f;
        player.velocity.x = clamp(player.velocity.x, -1000, 1000);
        player.position += player.velocity * 0.001f;
        if (player.position.y < -20){
            player.position.y  = -20;
            player.velocity.y = 0;
        }






        



        solver.draw_capsule(player.position, {10, 0}, {1, 0, 0});
        game->debugger.update_basic();
        game->debugger.draw(game->screen_pixel_size);
        game->end_main();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game->draw_main();
        SDL_GL_SwapWindow(window);
    }
    game->destroy();
    return 0;
}