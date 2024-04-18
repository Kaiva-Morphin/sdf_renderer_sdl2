#include "header.h"
#include "sdf_primitives.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <chrono>

#include <unordered_map>

#include <SDL2/SDL_ttf.h>
#include <stdio.h>

SDL_Window* window;
//SDL_Renderer* renderer;
SDL_GLContext context;
TTF_Font* font;

//480 * 270 ? 320 * 240
int TARGET_WIDTH = 320; // always true :party_popper:
int TARGET_HEIGHT = 240; // always true :party_popper:

float TARGET_ASPECT = (float)TARGET_WIDTH / (float)TARGET_HEIGHT;

class Shader{
    protected:
    GLuint program = glCreateProgram();
    string filePath;
    string current_src;
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
    virtual void compile(){};
    public:
    ivec2 size(){return ivec2(width, height);}
    
    void set_1f(const char* name, float value){
        glUniform1f(glGetUniformLocation(program, name), value);
    }
    void set_2f(const char* name, float value1, float value2){
        glUniform2f(glGetUniformLocation(program, name), value1, value2);
    }
};

class PassShader : public Shader {
    public:
    PassShader(){}
};


class Debugger{
    unordered_map<string, tuple<string, string>> lines; // todo: add destroying!
    vector<string> line_order; // todo: add destroying!
    void draw_line(int line, const char* text){
        SDL_Color textColor = {200, 200, 200}; // Black color
        SDL_Color bgColor = {30, 30, 30}; 
        
        SDL_Surface* textSurface = TTF_RenderText_Shaded(font, text, textColor, bgColor);
        if (textSurface == NULL) {
            printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
            return;
        }

        // Create texture from surface pixels
        SDL_Texture* texture = NULL;//SDL_CreateTextureFromSurface(renderer, textSurface);
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
        //SDL_RenderCopy(renderer, texture, &srcRect, &renderQuad);

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
        register_line(string("int_scale"), string("int_scale: "), string("?"));
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%ix%i", TARGET_WIDTH, TARGET_HEIGHT);
        register_line(string("resolution"), string("Current resolution: "), string(buffer));
        snprintf(buffer, sizeof(buffer), "%ix%i", TARGET_WIDTH, TARGET_HEIGHT);
        register_line(string("target_resolution"), string("Target resolution: "), string(buffer));
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
        if (lines.find(name) == lines.end()){ // if doesnt exists
            lines[name] = make_tuple(text, data);
            line_order.push_back(name);
        } else {
            get<1>(lines[name]) = data;
        }
    }
    void update_line_text(string name, string text){
        get<0>(lines[name]) = text;
    }
    void update_line(string name, string data){
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

double remap(double value, double fromLow, double fromHigh, double toLow, double toHigh) {
    double normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

class Game{
    public:
    bool running = true;
    bool true_scalling = false;
    ivec2 screen_pixel_size = ivec2(TARGET_WIDTH, TARGET_HEIGHT);
    ivec4 screen_rect = ivec4(0, 0, TARGET_WIDTH, TARGET_HEIGHT);
    vec4 uv_screen_rect = vec4(-1, -1, 1, 1);
    GLuint screen_texture = 0;
    GLuint screen_normalmap = 0;
    GLuint screen_depth = 0;
    GLuint framebuffer = 0;
    GLuint default_shader = 0;
    
    Debugger debugger;


    void init(){
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Simple Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");
        context = SDL_GL_CreateContext(window);
        //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        glewExperimental = GL_TRUE;
        glewInit();
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_TEXTURE_2D);

        debugger = Debugger();
        debugger.register_basic();
        
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glGenTextures(1, &screen_texture);
        update_resolution();

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_SetWindowResizable(window, SDL_bool::SDL_TRUE);
        SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");
        TTF_Init();
        font = TTF_OpenFont("assets/fonts/TinyUnicode.ttf", 16);

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendEquation(GL_BLEND_COLOR);

    }

    typedef void (*Function)(void);
    auto check_time(Function fn){
        auto start = chrono::high_resolution_clock::now();
        fn();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        return duration.count();
    }


    bool is_running(){
        return running;
    }

    float time(){
        return SDL_GetTicks() / 1000.0f;
    }
    

    void handle_event(SDL_Event e){
        if (e.type == SDL_KEYDOWN){
            if (e.key.keysym.sym == SDLK_F2){
                true_scalling = !true_scalling;
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

    void update_resolution(){ // mamma mia!
        debugger.update_line(string("int_scale"), string(true_scalling?"TRUE":"FALSE"));
        int current_window_w;
        int current_window_h;
        SDL_GetWindowSize(window, &current_window_w, &current_window_h);
        vec2 garanteed_rect;
        float aspect = (float)current_window_w / (float)current_window_h;
        if (aspect <= TARGET_ASPECT){
            garanteed_rect.x = (int)floor((float)current_window_w / (float)TARGET_WIDTH) * TARGET_WIDTH;
            if (garanteed_rect.x == 0 || !true_scalling)
                garanteed_rect.x = current_window_w;
            garanteed_rect.y = garanteed_rect.x / TARGET_ASPECT;
        }
        else {
            garanteed_rect.y = (int)floor((float)current_window_h / (float)TARGET_HEIGHT) * TARGET_HEIGHT;
            if (garanteed_rect.y == 0 || !true_scalling)
                garanteed_rect.y = current_window_h;
            garanteed_rect.x = garanteed_rect.y * TARGET_ASPECT;
        }
        float x_per_px = (float)garanteed_rect.x / (float)TARGET_WIDTH; // ~dpi
        float y_per_px = (float)garanteed_rect.y / (float)TARGET_HEIGHT;
        screen_rect.z = ceil(current_window_w / x_per_px) * x_per_px;
        screen_rect.w = ceil(current_window_h / y_per_px) * y_per_px;
        screen_rect.x = floor(((float)current_window_w - (float)screen_rect.z) / 2.0f);
        screen_rect.y = floor(((float)current_window_h - (float)screen_rect.w) / 2.0f);
        screen_pixel_size = {
            ceil(current_window_w / x_per_px),
            ceil(current_window_h / y_per_px)
        };
        if (screen_texture != 0)
            glDeleteTextures(1, &screen_texture);
        if (framebuffer != 0)
            glDeleteFramebuffers(1, &framebuffer);
        glGenTextures(1, &screen_texture);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_pixel_size.x, screen_pixel_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screen_texture, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%ix%i", screen_pixel_size.x, screen_pixel_size.y);
        debugger.update_line(string("resolution"), string(buffer));
    }
    ivec2 get_screen_size(){
        return screen_pixel_size;
    }

    void begin_main(){
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, screen_pixel_size.x, screen_pixel_size.y);
    }

    void end_main(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screen_rect.z, screen_rect.w);
    }
    void draw_main(){
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glBegin(GL_QUADS);
        glColor3f(1., 1., 1.);
        glTexCoord2f(0, 0); glVertex2f(uv_screen_rect.x, uv_screen_rect.y);
        glTexCoord2f(1, 0); glVertex2f(uv_screen_rect.z, uv_screen_rect.y);
        glTexCoord2f(1, 1); glVertex2f(uv_screen_rect.z, uv_screen_rect.w);
        glTexCoord2f(0, 1); glVertex2f(uv_screen_rect.x, uv_screen_rect.w);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void destroy(){
        if (screen_texture != 0) glDeleteTextures(1, &screen_texture);
        if (screen_depth != 0) glDeleteTextures(1, &screen_depth);
        if (screen_normalmap != 0) glDeleteTextures(1, &screen_normalmap);
        SDL_GL_DeleteContext(context);
        TTF_CloseFont(font);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

class SDF_Shader : public Shader{
    Debugger* debugger;
    GLuint scenebuffer;
    GLuint outputTexture;
    GLuint characterTexture;
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    SDL_Texture* sdl_output_texture = nullptr;
    int width;
    int height;
    
    public:
    
    SDF_Shader(string path, Debugger* debugger){
        filePath = path;
        this->debugger = debugger;
        this->debugger->register_line("SDFshader","SDF shader status: ","NOT INITED!");
    }
    
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
        glUniform1i(glGetUniformLocation(program, "character_texture"), 0);
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
        glAttachShader(program, computeShader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
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
        glDeleteTextures(1, &outputTexture);
        glDeleteTextures(1, &characterTexture);
        glDeleteBuffers(1, &scenebuffer);
        SDL_DestroyTexture(sdl_output_texture);
        glDeleteShader(computeShader);
        glDeleteProgram(program);
    }
    void use(){
        glUseProgram(program);
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
        if (sdl_output_texture != nullptr) SDL_DestroyTexture(sdl_output_texture);
        
        sdl_output_texture = nullptr;//SDL_CreateTextureFromSurface(renderer, surface);
        
        delete[] pixels;
        SDL_FreeSurface(surface);
        if (!sdl_output_texture) {
            printf("SDL Error: %s\n", SDL_GetError());
            return nullptr;
        }
        return sdl_output_texture;
    }



    void set_scene(PrimitiveScene* primitive_scene){
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, scenebuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(*primitive_scene), primitive_scene, GL_DYNAMIC_DRAW);
    }
};

