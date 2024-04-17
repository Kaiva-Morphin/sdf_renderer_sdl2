#include "Game.h"
#include "textre_drawer.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;


GLuint tilesetTexture;
GLuint shaderProgram;
GLuint VBO, VAO, EBO;

ivec2 texture_size;

void initTexture() {
    // Load tileset image and create texture
    SDL_Surface* surface = IMG_Load("assets/better_tiles.png");
    if (!surface) {
        std::cerr << "Failed to load tileset image: " << SDL_GetError() << std::endl;
        // Handle error
        return;
    }

    glGenTextures(1, &tilesetTexture);
    glBindTexture(GL_TEXTURE_2D, tilesetTexture);
    texture_size = {surface->w, surface->h};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    SDL_FreeSurface(surface);
}

void initShader() {
    // Vertex shader source code
    const char* vertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )glsl";

    // Fragment shader source code
    const char* fragmentShaderSource = R"glsl(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D texture1;
        void main() {
            FragColor = texture(texture1, TexCoord);
        }
    )glsl";

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void initBuffers() {
    // Define tile vertices, texture coordinates, and indices
    float vertices[] = {
        // Positions        // Texture coordinates
        0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Create VBO, VAO, and EBO
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);
}

void renderToTexture() {
    // Render tiles to texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Render to default framebuffer
    glBindTexture(GL_TEXTURE_2D, tilesetTexture);

    // Clear the texture
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render tiles here using the same rendering code as in render() function
    // Use shader program
    glUseProgram(shaderProgram);

    // Bind VAO
    glBindVertexArray(VAO);

    // Draw tiles
    // glDrawElements, glDrawArrays, etc.

    

    // Unbind VAO
    glBindVertexArray(0);
}

SDL_Texture* sdl_output_texture = nullptr;

SDL_Texture* get_texture(){
    GLubyte* pixels = new GLubyte[texture_size.x * texture_size.y * 4];
    glBindTexture(GL_TEXTURE_2D, tilesetTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels, texture_size.x, texture_size.y, 32, texture_size.x * 4, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        delete[] pixels;
        printf("SDL Error: %s\n", SDL_GetError());
        return nullptr;
    }
    if (sdl_output_texture != nullptr) SDL_DestroyTexture(sdl_output_texture);
    
    sdl_output_texture = SDL_CreateTextureFromSurface(renderer, surface);
    delete[] pixels;
    SDL_FreeSurface(surface);
    if (!sdl_output_texture) {
        printf("SDL Error: %s\n", SDL_GetError());
        return nullptr;
    }
    return sdl_output_texture;
}




SDL_Event event;

Game game = Game();
int main(int argc, char ** argv)
{
    game.init();
    
    IMG_Init(IMG_INIT_PNG);


    vector<vector<vector<int>>> map_data = {
        {
            {0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
        },
        {
            {0, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 5, 0, 0, 0, 1, 1},
            {0, 2, 1, 4, 0, 0, 3, 3},
            {0, 0, 3, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
        },
        {
            {1, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 3, 3},
            {0, 0, 1, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
        },
        {
            {1, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 3, 3},
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
        },
    };

    // "assets/better_tiles.png"

    initTexture();
    initShader();
    initBuffers();
    
    
    // todo: alpha checks for depth buffer draw
    while (game.is_running())
    {
        SDL_Delay(100);
        float time = game.time();
        game.debugger.update_basic();
        while (SDL_PollEvent(&event)) game.handle_event(event);
        game.switch_to_main();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        cout << endl;
        cout << "Render time: " << game.check_time(renderToTexture) << "ms" << endl;
        cout << "Get texture time: " << game.check_time([]() -> void {get_texture();}) << "ms" << endl;

        cout << "Copy time: " << game.check_time([]() -> void {SDL_RenderCopy(renderer, sdl_output_texture, nullptr, nullptr);}) << "ms" << endl;
        cout << "Debugger draw time: " <<  game.check_time([]() -> void {game.debugger.draw();}) << "ms" << endl;
        cout << "Apply main time: " << game.check_time([]() -> void {game.apply_main();}) << "ms" << endl;
        cout << "Present time: " << game.check_time([]() -> void {SDL_RenderPresent(renderer);}) << "ms" << endl;
        /*
            Render time: 6ms
            Copy time: 285ms
            Debugger draw time: 0ms
            Apply main time: 30ms
            Present time: 788ms
        */
    }
    IMG_Quit();
    game.destroy();
    return 0;
}