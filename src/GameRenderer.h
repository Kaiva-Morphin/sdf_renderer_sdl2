
#include "header.h"


#include "Shader.h"



#include <unordered_map>

#include <SDL2/SDL_ttf.h>
#include <stdio.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_GLContext context;
TTF_Font* font;

int TARGET_WIDTH = 320; // always true :party_popper:
int TARGET_HEIGHT = 240; // always true :party_popper:
float TARGET_ASPECT = (float)TARGET_WIDTH / (float)TARGET_HEIGHT;




class Debugger{
    unordered_map<string, tuple<string, string>> lines;

    void draw_line(int line, const char* text){
        SDL_Color textColor = {200, 200, 200}; // Black color
        SDL_Color bgColor = {30, 30, 30}; 
        
        SDL_Surface* textSurface = TTF_RenderText_Shaded(font, text, textColor, bgColor);
        if (textSurface == NULL) {
            printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
            return;
        }

        // Create texture from surface pixels
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (texture == NULL) {
            printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
            return;
        }

        // Get width and height of text surface
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        int offset = 5;
        // Set rendering space and render to screen
        SDL_Rect renderQuad = {0, (textHeight - offset - 1) * line, textWidth, textHeight - offset};
        SDL_Rect srcRect = {0, offset, textWidth, textHeight};
        SDL_RenderCopy(renderer, texture, &srcRect, &renderQuad);

        // Cleanup
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(texture);
    }
    int startTime = SDL_GetTicks();
    int endTime = 0;
    int frameCount = 0;
    public:
    void register_basic(){
        register_line(string("fps"), string("FPS: "), string("?"));
        register_line(string("ticks"), string("tick: "), string("?"));
    }
    void update_basic(){
        char buffer[20];
        endTime = SDL_GetTicks();
        frameCount++;
        if (endTime - startTime >= 200) {
            float fps = frameCount / ((endTime - startTime) / 1000.0f);
            snprintf(buffer, sizeof(buffer), "%i", (int)round(fps));
            update_line(string("fps"), string(buffer));

            // Reset timing variables
            startTime = endTime;
            frameCount = 0;
        }
        snprintf(buffer, sizeof(buffer), "%u", endTime);
        update_line(string("ticks"), string(buffer));
    }

    void register_line(string name, string text, string data){
        lines[name] = make_tuple(text, data);
    }
    void update_line_text(string name, string text){
        get<0>(lines[name]) = text;
    }
    void update_line(string name, string data){
        auto& ref = lines[name];
        get<1>(lines[name]) = data;
    }
    void destroy_line(string name){
        lines.erase(name);
    }
    void draw(){
        int line = 0;
        for (const auto& pair : lines) {
            string text;
            string data;
            tie(text, data) = pair.second;
            draw_line(line, (text + data).c_str());
            line += 1;
        }
    };
};


class GameRenderer{ // scale?
    bool running = true;
    bool true_scaling = false;
    SDL_Rect bg_rect;
    SDL_Rect main_rect;
    SDL_Texture* bg_texture;
    SDL_Texture* main_texture;
    public:
    Debugger debugger;
    void init(){
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Simple Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
        context = SDL_GL_CreateContext(window);
        glewExperimental = GL_TRUE;
        glewInit();
        glEnable( GL_DEBUG_OUTPUT );
        debugger = Debugger();
        debugger.register_line(string("int_scale"), string("int_scale: "), string("true"));
        update_resolution();

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        //SDL_RenderSetLogicalSize(renderer, TARGET_WIDTH, TARGET_HEIGHT);
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
        if (true_scaling) SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_TRUE);
        else SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_FALSE);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_SetWindowResizable(window, SDL_bool::SDL_TRUE);
        //SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, 0);
        SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        TTF_Init();
        font = TTF_OpenFont("assets/fonts/TinyUnicode.ttf", 16);
        main_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, TARGET_WIDTH, TARGET_HEIGHT);
    }

    bool is_running(){
        return running;
    }

    void handle_event(SDL_Event e){
        if (e.type == SDL_KEYDOWN){
            if (e.key.keysym.sym == SDLK_F2){
                true_scaling = !true_scaling;
                if (true_scaling) SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_TRUE);
                else SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_FALSE);
                update_resolution();
            }
        }
        

        if (e.type == SDL_QUIT) {
            running = false;
        }
        else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                update_resolution();
            }
        }
    }

    void update_resolution(){
        debugger.update_line(string("int_scale"), string(true_scaling?"TRUE":"FALSE"));
        int current_window_w;
        int current_window_h;
        SDL_GetWindowSize(window, &current_window_w, &current_window_h);
        float aspect = (float)current_window_w / (float)current_window_h;
        if (aspect < TARGET_ASPECT){
            if (true_scaling){
                main_rect.w = (int)floor((float)current_window_w / (float)TARGET_WIDTH) * TARGET_WIDTH;
                if (main_rect.w == 0){
                    main_rect.w = current_window_w;
                    SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_FALSE);
                } else {
                    SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_TRUE);
                }
            } else main_rect.w = current_window_w;
            main_rect.h = main_rect.w / TARGET_ASPECT;
        }
        else {
            if (true_scaling){
                main_rect.h = (int)floor((float)current_window_h / (float)TARGET_HEIGHT) * TARGET_HEIGHT;
                if (main_rect.h == 0){
                    main_rect.h = current_window_h;
                    SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_FALSE);
                } else {
                    SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_TRUE);
                }
            } else main_rect.h = current_window_h;
            main_rect.w = main_rect.h * TARGET_ASPECT;
        }
        main_rect.x = (current_window_w - main_rect.w) / 2;
        main_rect.y = (current_window_h - main_rect.h) / 2;
    }

    void switch_to_main(){
        SDL_SetRenderTarget(renderer, main_texture);
    }

    void appy_main(){
        SDL_SetRenderTarget(renderer, nullptr);
        
        SDL_RenderCopy(renderer, main_texture, nullptr, &main_rect);
    }

    void destroy(){
        SDL_GL_DeleteContext(context);
        TTF_CloseFont(font);
        SDL_DestroyTexture(main_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};









