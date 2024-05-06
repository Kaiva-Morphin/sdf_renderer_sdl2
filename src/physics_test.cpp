#include "Game.h"
#include "Physics.h"

#include "BdfFont.h"




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
    BDFAtlas font_atlas = BDFAtlas("assets/fonts/orp/orp-book.bdf", 1536);
    game.debugger.init(&font_atlas);
    game.debugger.register_basic();
    PhysicsSolver physics = PhysicsSolver("assets/shaders/physics.comp");
    physics.init();



    PhysicsPrimitive cursor = physics.capsule(0., 40.);
    cursor.type = RIGID;
    physics.push(&cursor);

    /*PhysicsPrimitive line1 = physics.line(vec3(25., 25., 0.), vec3(-25., -25., 0.));
    physics.push(&line1);
    line1.position.x = 125;
    line1.position.y = -75;

    PhysicsPrimitive line3 = physics.line(vec3(-25., -25., 0.), vec3(25., 25., 0.));
    line3.position.x = 125;
    line3.position.y = 0;
    physics.push(&line3);*/

    PhysicsPrimitive line2 = physics.line(vec3(50., 0., 0.), vec3(-50., 0., 0.));
    line2.position.y = -0;
    physics.push(&line2);


    int xMouse, yMouse;
    while (game.is_running())
    {
        Uint32 mouseState = SDL_GetMouseState(&xMouse,&yMouse);
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        vec2 half_screen = vec2(game.screen_pixel_size) * 0.5f;
        vec2 target = {
            remap(xMouse, 0, w, -half_screen.x,  half_screen.x),
            remap(yMouse, 0, h,  half_screen.y, -half_screen.y)
        };

        if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            cursor.type = RIGID;
            cursor.shape = CAPSULE;
            cursor.velocity = (vec4(target, 0, 0) - cursor.position);// * 100.0f;
            cursor.rounding = 40;
        } else {
            //cursor.type = MOVING;
            cursor.position = vec4(target, 0, 0);
            cursor.rounding = 40;
        }

        float time = game.time();
        while (SDL_PollEvent(&event)) game.handle_event(event);

        game.begin_main();
        glClearColor(0.7843, 0.7333, 0.5882, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        physics.check_file_updates();
        
        physics.step(0.01, game.screen_pixel_size, &font_atlas);
        physics.draw(game.screen_pixel_size);

        //physics.lines(game.screen_pixel_size, &font_atlas);







        //physics.draw_box(vec2(0.), vec2(10.), game.screen_pixel_size, vec3(1., 0., 0.));
        //physics.draw_capsule(vec2(0., 0.), vec2(6., 30.), game.screen_pixel_size, vec3(1., 0., 0.));
        game.debugger.update_basic();
        game.debugger.draw(game.screen_pixel_size);
        game.end_main();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game.draw_main();
        SDL_GL_SwapWindow(window);
    }
    game.destroy();
    return 0;
}