#include "header.h"
#include "sdf_primitives.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <unordered_map>

#include <SDL2/SDL_ttf.h>
#include <stdio.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_GLContext context;
TTF_Font* font;

//480 * 270 ? 320 * 240
int TARGET_WIDTH = 320; // always true :party_popper:
int TARGET_HEIGHT = 240; // always true :party_popper:
float TARGET_ASPECT = (float)TARGET_WIDTH / (float)TARGET_HEIGHT;

//SDL_GetError()


class Debugger{
    unordered_map<string, tuple<string, string>> lines;
    vector<string> line_order;
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
        SDL_Rect renderQuad = {0, (textHeight - offset) * line, textWidth, textHeight - offset};
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
    bool enabled = false;
    void register_basic(){
        register_line(string("fps"), string("FPS: "), string("?"));
        register_line(string("ticks"), string("tick: "), string("?"));
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%ix%i", TARGET_WIDTH, TARGET_HEIGHT);
        register_line(string("resolution"), string("Resolution: "), string(buffer));
        int x, y;
        SDL_GetWindowSize(window, &x, &y);
        snprintf(buffer, sizeof(buffer), "%ix%i", x, y);
        register_line(string("window_size"), string("Window size: "), string(buffer));
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
        line_order.push_back(name);
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
        if (!enabled){return;}
        int line = 0;
        for (const auto& line_name : line_order) {
            string text;
            string data;
            tie(text, data) = lines[line_name];
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
        window = SDL_CreateWindow("Simple Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
        context = SDL_GL_CreateContext(window);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        glewExperimental = GL_TRUE;
        glewInit();
        glEnable( GL_DEBUG_OUTPUT );

        debugger = Debugger();
        debugger.register_line(string("int_scale"), string("int_scale: "), string("true"));
        update_resolution();

        //SDL_RenderSetLogicalSize(renderer, TARGET_WIDTH, TARGET_HEIGHT);
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
            if (e.key.keysym.sym == SDLK_F3){
                debugger.enabled = !debugger.enabled;
            }
        }
        if (e.type == SDL_QUIT) {
            running = false;
        }
        else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                update_resolution();
                int x, y;
                char buffer[20];
                SDL_GetWindowSize(window, &x, &y);
                snprintf(buffer, sizeof(buffer), "%ix%i", x, y);
                debugger.update_line(string("window_size"), string(buffer));
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



class Shader{};


class SDF_Shader : public Shader{
    string filePath;
    string current_src;
    Debugger* debugger;
    GLuint scenebuffer;
    GLuint outputTexture;
    GLuint characterTexture;
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    SDL_Texture* sdl_output_texture = nullptr;
    int width;
    int height;
    string read_shader(){
        ifstream fileStream(filePath);
        if (!fileStream.is_open()) {
            cerr << "ERROR! Cant open file!" << endl;
            return "";
        }
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }
    public:
    GLuint computeProgram = glCreateProgram();
    SDF_Shader(string path, Debugger* debugger){
        filePath = path;
        this->debugger = debugger;
        this->debugger->register_line("SDFshader","SDF shader status: ","NOT INITED!");
    }
    ivec2 size(){return ivec2(width, height);}
    void init(int w, int h, ivec3 binded_texture_size, GLubyte* binded_texture_data){
        this->debugger->register_line("SDFshader","SDF shader status: ","Initing...");
        width = w;
        height = h;
        current_src = read_shader();
        glGenTextures(1, &outputTexture);
        glBindTexture(GL_TEXTURE_2D, outputTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenTextures(1, &characterTexture);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, binded_texture_size.x, binded_texture_size.y, binded_texture_size.z, 0, GL_RGB, GL_UNSIGNED_BYTE, binded_texture_data);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
        
        compile();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glUniform1i(glGetUniformLocation(computeProgram, "character_texture"), 0);
    }
    void compile(){
        this->debugger->register_line("SDFshader","SDF shader status: ","Compiling...");
        const GLchar* shaderSource = current_src.c_str();
        glShaderSource(computeShader, 1, &shaderSource, NULL);
        glCompileShader(computeShader);
        GLint success;
        glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
            std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
            this->debugger->update_line("SDFshader", string("compilation failed! check console"));
            return;
        }
        glAttachShader(computeProgram, computeShader);
        glLinkProgram(computeProgram);
        glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(computeProgram, 512, NULL, infoLog);
            std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
            this->debugger->update_line("SDFshader", string("program linking failed! check console"));
            return;
        }
        glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

        glGenBuffers(1, &scenebuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scenebuffer);
        
        this->debugger->register_line("SDFshader","SDF shader status: ","Compiled!");
    }
    void destroy(){
        glDeleteBuffers(1, &scenebuffer);
        SDL_DestroyTexture(sdl_output_texture);
        glDeleteShader(computeShader);
        glDeleteProgram(computeProgram);
    }
    void use(){
        glUseProgram(computeProgram);
    }
    
    void check_file_updates(){
        string new_src = read_shader();
        if (new_src != current_src){
            current_src = new_src;
            compile();
        }
    }

    void run(){
        glDispatchCompute(ceil((float)width / 16.), ceil((float)height / 16.), 1);
    }
    void wait(){
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    }
    SDL_Texture* get_texture(){
        GLubyte* pixels = new GLubyte[width * height * 4];
        glBindTexture(GL_TEXTURE_2D, outputTexture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, 32, width * 4, SDL_PIXELFORMAT_RGBA32);
        if (!surface) {
            delete[] pixels;
            printf("SDL Error: %s\n", SDL_GetError());
            return nullptr;
        }
        if (sdl_output_texture != nullptr){
            SDL_DestroyTexture(sdl_output_texture);
        };
        
        sdl_output_texture = SDL_CreateTextureFromSurface(renderer, surface);
        delete[] pixels;
        SDL_FreeSurface(surface);
        if (!sdl_output_texture) {
            printf("SDL Error: %s\n", SDL_GetError());
            return nullptr;
        }
        return sdl_output_texture;
    }

    void set_1f(const char* name, float value){
        glUniform1f(glGetUniformLocation(computeProgram, name), value);
    }
    void set_2f(const char* name, float value1, float value2){
        glUniform2f(glGetUniformLocation(computeProgram, name), value1, value2);
    }

    void set_scene(PrimitiveScene* primitive_scene){
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, scenebuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(*primitive_scene), primitive_scene, GL_DYNAMIC_DRAW);
    }
};

