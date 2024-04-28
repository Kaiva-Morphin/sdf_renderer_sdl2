#include "Game.h"
#include "Physics.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;

Game game = Game();

//*     Y
//*     |
//*     * -- X
//*      \
//*        Z

int main(int argc, char ** argv)
{
    game.init();
    
    PhysicsSolver physics = PhysicsSolver("assets/shaders/physics.comp");
    physics.init();


    PhysicsPrimitive box1 = physics.box(vec3(100., 100., 100.));
    box1.position.y = 50;
    box1.position.x = 50;
    physics.push(&box1);

    PhysicsPrimitive caps1 = physics.capsule(25, 20);
    caps1.position.y = 50;
    caps1.position.x = -50;
    physics.push(&caps1);

    PhysicsPrimitive caps2 = physics.capsule(24, 12);
    caps2.position.y = 0;
    caps2.position.x =-0;
    caps2.type = RIGID;
    physics.push(&caps2);


    PhysicsPrimitive floor = physics.box(vec3(200., 10., 200.));
    floor.position.y = -100;
    physics.push(&floor);


    PhysicsPrimitive player = physics.capsule(8, 8);
    player.type = RIGID;
    physics.push(&player);

    int xMouse, yMouse;
    while (game.is_running())
    {
        Uint32 mouseState = SDL_GetMouseState(&xMouse,&yMouse);
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        vec2 half_screen = vec2(game.screen_pixel_size) * 0.5f;
        player.position.x = remap(xMouse, 0, w, -half_screen.x,  half_screen.x);
        player.position.y = remap(yMouse, 0, h,  half_screen.y, -half_screen.y);

        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            player.type = RIGID;
        } else {
            player.type = MOVING;
        }

        float time = game.time();
        while (SDL_PollEvent(&event)) game.handle_event(event);

        game.begin_main();
        glClearColor(0.7843, 0.7333, 0.5882, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        physics.check_file_updates();
        
        physics.step(0.01, game.screen_pixel_size);
        
        physics.draw(game.screen_pixel_size);
        //physics.draw_box(vec2(0.), vec2(10.), game.screen_pixel_size, vec3(1., 0., 0.));
        //physics.draw_capsule(vec2(0., 0.), vec2(6., 30.), game.screen_pixel_size, vec3(1., 0., 0.));
        
        
        game.end_main();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game.draw_main();
        SDL_GL_SwapWindow(window);
    }
    game.destroy();
    return 0;
}