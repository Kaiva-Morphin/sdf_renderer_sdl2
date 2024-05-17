#include "Game.h"
#include "Physics.h"
#include "BdfFont.h"




#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;

//*     Y
//*     |
//*     * -- X
//*      \
//*        Z

int main(int argc, char ** argv)
{
    Game g = Game();
    game = &g;
    game->init();
    BDFAtlas font_atlas = BDFAtlas("assets/fonts/orp/orp-book.bdf", 1536);
    game->debugger.init(&font_atlas);
    game->debugger.register_basic();
    PhysicsSolver physics = PhysicsSolver("assets/shaders/physics.comp");
    physics.init();



    PhysicsPrimitive cursor = physics.capsule(40., 20.);
    cursor.type = TYPE_RIGID;
    cursor.bounciness = 0.0;
    //cursor.mass = 100.;
    cursor.friction = 0.5;
    //physics.push(&cursor);

    //PhysicsPrimitive circ = physics.capsule(0., 10.);
    //circ.type = TYPE_RIGID;
    //circ.position.x = 60;
    ////circ.bounciness = 0.9;
    ////circ.friction = 0.99;
    //physics.push(&circ);
//
    //PhysicsPrimitive circ1 = physics.capsule(0., 10.);
    //circ1.position.x = -60;
    ////circ1.bounciness = 0.9;
    ////circ1.friction = 0.99;
    //circ1.type = TYPE_RIGID;
    //physics.push(&circ1);

    /*{
        PhysicsPrimitive p;
        p.a = vec4(0);
        p.rounding = 10;
        p.bounciness = 0.99;
        p.friction = 0.99;
        p.shape = SHAPE_CAPSULE;
        p.type = TYPE_RIGID;
        for (int i = 0; i < 5; i++){
            p.position.x = rand() % 300 - 150;
            p.position.y = rand() % 300 - 150;
            p.velocity.x = rand() % 200 - 100;
            p.velocity.y = rand() % 200 - 100;
            PhysicsPrimitive* temp = new PhysicsPrimitive{p};
            physics.push(temp);
        }
    }*/

    /*PhysicsPrimitive line1 = physics.line(vec3(50., 0, 0.), vec3(-50., 0, 0.));
    line1.position.x = 100;
    line1.position.y = 0;
    physics.push(&line1);
    PhysicsPrimitive line2 = physics.line(vec3(50., 0, 0.), vec3(40., 60, 0.));
    line2.position.x = -100;
    line2.position.y = 0;
    physics.push(&line2);
    PhysicsPrimitive line3 = physics.line(vec3(50., 0, 0.), vec3(-50., 0, 0.));
    line3.position.x = 0;
    line3.position.y = 0;
    physics.push(&line3);*/

    /*PhysicsPrimitive line1 = physics.line(vec3(50., -50, 0.), vec3(-50., -25, 0.));
    line1.position.x = 1;
    line1.position.y = 1;
    physics.push(&line1);
    PhysicsPrimitive line2 = physics.line(vec3(50., 25, 0.), vec3(50., -25, 0.));
    line2.position.x = 1;
    line2.position.y = 1;
    physics.push(&line2);*/


    /*PhysicsPrimitive line2 = physics.line(vec3(50., 25., 0.), vec3(-50., -25., 0.));
    physics.push(&line2);
    line2.position.x = 100;
    line2.position.y = -60;*/


    /*PhysicsPrimitive line3 = physics.line(vec3(25., 25., 0.), vec3(-25., -25., 0.));
    physics.push(&line3);
    line3.position.x = 125;
    line3.position.y = -75;

    PhysicsPrimitive line4 = physics.line(vec3(25., 25., 0.), vec3(-25., -25., 0.));
    physics.push(&line4);
    line4.position.x = 125;
    line4.position.y = -75;*/

    /*for (int side = 0; side < 4; side++) {
        PhysicsPrimitive p;
        switch (side)
        {
        case 0:
            p.a = vec4( 10, 0, 0, 0);
            p.b = vec4(-10, 0, 0, 0);
            break;
        case 1:
            p.a = vec4( 0, -10, 0, 0);
            p.b = vec4( 0,  10, 0, 0);
            break;
        case 2:
            p.a = vec4(-10, 0, 0, 0);
            p.b = vec4( 10, 0, 0, 0);
            break;
        case 3:
            p.a = vec4( 0,  10, 0, 0);
            p.b = vec4( 0, -10, 0, 0);
            break;
        }
        p.shape = SHAPE_LINE;
        p.normal = physics.normal_of_line(p.a, p.b);
        if (side == 0 || side == 2) {
            for (int x = -7; x <= 7; x++){
                p.position = vec4(x * 20, side == 0 ? -120 : 120, 0, 0);
                PhysicsPrimitive* temp = new PhysicsPrimitive{p};
                physics.push(temp);
            }
        }
        if (side == 1 || side == 3) {
            for (int y = -6; y <= 6; y++){
                p.position = vec4(side == 1?-150:150, y * 20, 0, 0);
                PhysicsPrimitive* temp = new PhysicsPrimitive{p};
                physics.push(temp);
            }
        }
    }*/


    PhysicsPrimitive player = physics.capsule(40., 20.);
    player.type = TYPE_RIGID;
    player.bounciness = 0.5;
    player.mass = 1.;
    player.position.x = 100;
    player.friction = 0.5;
    physics.push(&player);


    vec3 pos = vec3(0, 0, 0);
    vec3 half_size = vec3(0.5, 0.5, 0.5);
    vec3 right_up_near =   half_size * vec3( 1,  1,  1);
    vec3 right_up_far =    half_size * vec3( 1,  1, -1);
    vec3 right_down_near = half_size * vec3( 1, -1,  1);
    vec3 right_down_far =  half_size * vec3( 1, -1, -1);
    vec3 left_up_near =    half_size * vec3(-1,  1,  1);
    vec3 left_up_far =     half_size * vec3(-1,  1, -1);
    vec3 left_down_near =  half_size * vec3(-1, -1,  1);
    vec3 left_down_far =   half_size * vec3(-1, -1, -1);


    PhysicsPrimitive *up1 = new PhysicsPrimitive;
    *up1 = physics.triangle(pos, right_up_near, right_up_far, left_up_near);
    physics.push(up1);
    PhysicsPrimitive *up2 = new PhysicsPrimitive;
    *up2 = physics.triangle(pos, left_up_far, left_up_near, right_up_far);
    physics.push(up2);

    PhysicsPrimitive *down1 = new PhysicsPrimitive;
    *down1 = physics.triangle(pos, right_down_near, left_down_near, right_down_far);
    physics.push(down1);
    PhysicsPrimitive *down2 = new PhysicsPrimitive;
    *down2 = physics.triangle(pos, left_down_far, right_down_far, left_down_near);
    physics.push(down2);

    PhysicsPrimitive *front1 = new PhysicsPrimitive;
    *front1 = physics.triangle(pos, right_up_near, left_up_near, right_down_near);
    physics.push(front1);
    PhysicsPrimitive *front2 = new PhysicsPrimitive;
    *front2 = physics.triangle(pos, left_down_near, right_down_near, left_up_near);
    physics.push(front2);

    PhysicsPrimitive *back1 = new PhysicsPrimitive;
    *back1 = physics.triangle(pos, right_up_far, right_down_far, left_up_far);
    physics.push(back1);
    PhysicsPrimitive *back2 = new PhysicsPrimitive;
    *back2 = physics.triangle(pos, left_down_far, left_up_far, right_down_far);
    physics.push(back2);

    PhysicsPrimitive *left1 = new PhysicsPrimitive;
    *left1 = physics.triangle(pos, left_down_near, left_up_near, left_down_far);
    physics.push(left1);
    PhysicsPrimitive *left2 = new PhysicsPrimitive;
    *left2 = physics.triangle(pos, left_up_far, left_down_far, left_up_near);
    physics.push(left2);

    PhysicsPrimitive *right1 = new PhysicsPrimitive;
    *right1 = physics.triangle(pos, right_down_near, right_down_far, right_up_near);
    physics.push(right1);
    PhysicsPrimitive *right2 = new PhysicsPrimitive;
    *right2 = physics.triangle(pos, right_up_far, right_up_near, right_down_far);
    physics.push(right2);

    bool key_a, key_d, key_w, key_s, key_space, key_shift;
    key_a = false;
    key_d = false;
    key_w = false;
    key_s = false;
    key_space = false;
    key_shift = false;

    PhysicsPrimitive *t1 = new PhysicsPrimitive;
    *t1 = physics.triangle(pos, vec3(rand()%80, rand()%80, 0), vec3(rand()%80, rand()%80, 0), vec3(rand()%80, rand()%80, 0));
    t1->a = vec4(0, 0, 0, 0);
    t1->b = vec4(30, 0, 0, 0);
    t1->c = vec4(0, 30, 0, 0);
    physics.push(t1);
    PhysicsPrimitive *t2 = new PhysicsPrimitive;
    *t2 = physics.triangle(pos, vec3(rand()%80, rand()%80, 0), vec3(rand()%80, rand()%80, 0), vec3(rand()%80, rand()%80, 0));
    t2->a = vec4(0, 0, 0, 0);
    t2->b = vec4(30, 0, 0, 0);
    t2->c = vec4(0, 30, 0, 0);
    physics.push(t2);

    int xMouse, yMouse;
    while (game->is_running())
    {
        Uint32 mouseState = SDL_GetMouseState(&xMouse,&yMouse);
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        vec2 target = {
            remap(xMouse, 0, w, -half_screen.x,  half_screen.x),
            remap(yMouse, 0, h,  half_screen.y, -half_screen.y)
        };

        if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            cursor.type = TYPE_RIGID;
            cursor.shape = SHAPE_CAPSULE;
            cursor.velocity = (vec4(target, 0, 0) - cursor.position);// * 100.0f;
            player.position = vec4(target, 0, 0);
        } else {
            //cursor.type = MOVING;
            cursor.position = vec4(target, 0, 0);
        }




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
                        key_w = true;
                        break;
                    case SDLK_s:
                        key_s = true;
                        break;
                    case SDLK_SPACE:
                        key_space = true;
                        break;
                    case SDLK_LSHIFT:
                        key_shift = true;
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
                    case SDLK_SPACE:
                        key_space = false;
                        break;
                    case SDLK_LSHIFT:
                        key_shift = false;
                        break;
                }
            }
        }

        game->begin_main();
        glClearColor(0.7843, 0.7333, 0.5882, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);


        /*vec4 start = vec4(-100, -100, 0, 0);//cursor.position;
        start = cursor.position;
        vec4 end = vec4(100, 100, 0, 0);//cursor.position + cursor.velocity;
        end = cursor.position + cursor.velocity;

        vec4 vertex1 = {-50, -25, 0, 0};
        vec4 vertex2 = {50, 25, 0, 0};
        float radius = 20;
        float height = 40;

        vec4 normal_of_line = physics.normal_of_line(vertex1, vertex2);

        physics.draw_line(vertex1, vertex2, vec3{1, 0, 0});
        physics.draw_capsule(start, {radius, height}, {1, 1, 0});
        physics.draw_line(start, end, {1, 1, 0});*/

        /*sgame->start_timer();
        for (int i = 0; i < 100000; i++){
            physics.math_step(start, end, vertex1, vertex2, normal_of_line, radius);
        }
        printf("Math: ");
        game->print_timer_end();
        game->start_timer();
        for (int i = 0; i < 100000; i++){
            physics.binsearch_step(start, end, vertex1, vertex2, normal_of_line, radius, height);
        }
        printf("Sosiska: ");
        game->print_timer_end();*/

        //vec4 res = physics.binsearch_step(start, end, vertex1, vertex2, normal_of_line, radius, height);
        //physics.draw_capsule(res, {radius, height}, {1, 1, 0});
        //res = physics.math_step(start, end, vertex1, vertex2, normal_of_line, radius);
        //physics.draw_capsule(res, {radius, 0}, {1, 0, 1});
        //physics.check_file_updates();
        //physics.step(game->wrapped_delta());
        //physics.draw();

        //physics.lines(game->screen_pixel_size, &font_atlas);

        //physics.draw_box(vec2(0.), vec2(10.), game->screen_pixel_size, vec3(1., 0., 0.));
        //physics.draw_capsule(vec2(0., 0.), vec2(6., 30.), game->screen_pixel_size, vec3(1., 0., 0.));
        key_w = false;
        key_s = false;
        vec2 target_vel = vec2(key_d - key_a,  key_s - key_w) * 16.0f;
        player.velocity.z = target_vel.y;
        player.velocity.x = target_vel.x;
        player.velocity.y = (key_space - key_shift) * 16.0f;
        
        

        game->debugger.register_line("phys", "collide:  ", "");
        game->debugger.register_line("phys0", "normal:  ", "");
        game->debugger.register_line("phys1", "phys:  ", "");
        game->debugger.register_line("phys2", "phys:  ", "");

        physics.step(game->wrapped_delta() * 10, &font_atlas);


        t1->position.x = sin(time*5)*100;
        t2->position.y = sin(time*2.5)*100;

        physics.draw();

        float d;
        vec3 a;
        vec3 b;
        //t2->position = vec4(100, 100, 0, 0);
        tie(d, a, b) = physics.triangle_triangle(t1->position+t1->a, t1->position+t1->b, t1->position+t1->c, t2->position+t2->a, t2->position+t2->b, t2->position+t2->c);
        game->debugger.update_line("phys1", "A: " + to_string(a));
        game->debugger.update_line("phys2", "B: " + to_string(a));
        physics.draw_line(a, b, {1, 1, 0});




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