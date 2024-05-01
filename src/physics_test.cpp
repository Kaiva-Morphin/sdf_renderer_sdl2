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

    vec3 grid_size = vec3(40, 20, 40);
    int map_size = 5;

    PhysicsPrimitive box = physics.box(grid_size);
    PhysicsPrimitive slope = physics.pyramid(vec3(grid_size.x, -grid_size.y*0.5f, 0.),vec3(grid_size.x*0.5f, grid_size.y*0.5f, 0.));
    PhysicsPrimitive slope_inv = physics.pyramid(vec3(grid_size.x, -grid_size.y*0.5f, 0.),vec3(-grid_size.x*0.5f, grid_size.y*0.5f, 0.));
    for (int x = -map_size; x <= map_size; x++){
        for (int y = -map_size; y <= map_size; y++){
            if (y == -map_size || x == -map_size || x == map_size){
                PhysicsPrimitive* new_box = new PhysicsPrimitive(box);
                new_box->position.x = x * grid_size.x;
                new_box->position.y = y * grid_size.y;
                physics.push(new_box);
            }
            if (x-4 == y && y > -map_size){
                PhysicsPrimitive* new_slope = new PhysicsPrimitive(slope);
                new_slope->position.x = x * grid_size.x;
                new_slope->position.y = y * grid_size.y;
                physics.push(new_slope);
            }


        }
    }



    PhysicsPrimitive capsule = physics.capsule(32, 16);
    capsule.type = RIGID;
    capsule.position.x = -40;
    capsule.position.y = -10;
    capsule.y_slopes = false;
    capsule.bounciness = 1.;
    capsule.friction = 0.99;
    physics.push(&capsule);



    PhysicsPrimitive player = physics.capsule(32, 16);
    player.type = RIGID;
    player.bounciness = 1.;
    player.friction = 0.99;
    player.y_slopes = true;
    physics.push(&player);


    PhysicsPrimitive cursor = physics.box(vec3(20., 20., 20.));
    cursor.type = RIGID;
    
    physics.push(&cursor);

    int xMouse, yMouse;
    while (game.is_running())
    {
        Uint32 mouseState = SDL_GetMouseState(&xMouse,&yMouse);
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        vec2 half_screen = vec2(game.screen_pixel_size) * 0.5f;
        cursor.position.x = remap(xMouse, 0, w, -half_screen.x,  half_screen.x);
        cursor.position.y = remap(yMouse, 0, h,  half_screen.y, -half_screen.y);

        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            cursor.type = RIGID;
            cursor.shape = CAPSULE;
            cursor.velocity = vec4(10., 10., 0., 0.);
            cursor.rounding = 10;
        } else {
            cursor.type = MOVING;
            cursor.shape = BOX;
            cursor.rounding = 0;
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