#include "Game.h"
#include "textre_drawer.h"

using namespace glm;

SDL_Event event;
Game game = Game();
int main(int argc, char ** argv)
{
    game.init();
    
    IMG_Init(IMG_INIT_PNG);



    while (game.is_running())
    {
        float time = game.time();
        while (SDL_PollEvent(&event))
        {
            game.handle_event(event);
        }
        
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_BLEND);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_BLEND_COLOR);
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game.switch_to_main();
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
        glColor4f(1, 0, 1, 1);
        glVertex3f(0, 1, 0);
        glColor4f(0, 1, 1, 1);
        glVertex3f(-1, -1, 0);
        glColor4f(1, 1, 0, 1);
        glVertex3f(1, -1, 0);
        glEnd();
        game.apply_main();
        SDL_GL_SwapWindow(window);
    }
    IMG_Quit();
    game.destroy();
    return 0;
}