#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cmath>
#include <iostream>

SDL_Window* window;
SDL_GLContext context;
GLuint textureID;
SDL_Texture* sdlTexture;
SDL_Renderer* renderer;

const int TARGET_WIDTH = 240;
const int TARGET_HEIGHT = 144;
const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

void initSDL() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // Handle initialization error
    }

    // Set OpenGL attributes before creating the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    

    // Create SDL window
    window = SDL_CreateWindow("OpenGL to SDL Texture", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
    if (!window) {
        // Handle window creation error
    }
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
    SDL_SetWindowResizable(window, SDL_bool::SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, TARGET_WIDTH, TARGET_HEIGHT);



    // Create OpenGL context
    context = SDL_GL_CreateContext(window);
    if (!context) {
        // Handle context creation error
    }
}

void initTexture() {
    // Generate OpenGL texture
	textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    // Set texture parameters for lower resolution (adjust as needed)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create SDL texture at a lower resolution
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 100, 100);

    // Bind SDL texture to OpenGL context
    std::cout << SDL_GL_BindTexture(sdlTexture, NULL, NULL);
}

void renderToTexture() {
    // Clear the framebuffer
	
	SDL_GL_BindTexture(sdlTexture, NULL, NULL);
	glSelectBuffer(GLsizei{100 * 100}, &textureID);
	glViewport(0, 0, 100, 100);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
	
    // Set up an orthographic projection for lower resolution (adjust as needed)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 100, 100, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw a circle
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 1.0f); // Set circle color to white

    float centerX = 50.0f;
    float centerY = 50.0f;
    float radius = 50.0f;
    int numSegments = 5;

    for (int i = 0; i <= numSegments; ++i) {
        float theta = (2.0f * M_PI * static_cast<float>(i)) / static_cast<float>(numSegments);
        float x = radius * std::cos(theta) + centerX;
        float y = radius * std::sin(theta) + centerY;

        glVertex2f(x, y);
    }

    glEnd();
	GLubyte* pixels = new GLubyte[100*100*4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	GLuint r, g, b, a; // or GLubyte r, g, b, a;

	size_t x, y; // line and column of the pixel
	x = 50;
	y = 50;
	size_t elmes_per_line = 100 * 4; // elements per line = 256 * "RGBA"

	size_t row = y * elmes_per_line;
	size_t col = x * 4;

	r = pixels[row + col]; 
	g = pixels[row + col + 1]; 
	b = pixels[row + col + 2]; 
	a = pixels[row + col + 3]; 
	std::cout << g << std::endl;
    // Unbind SDL texture from OpenGL context
    SDL_GL_UnbindTexture(sdlTexture);
    // Update SDL texture
    SDL_UpdateTexture(sdlTexture, NULL, nullptr, 100 * sizeof(Uint32));

    // Render SDL texture
    SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
    // Rebind SDL texture to OpenGL context for the next frame
    
}

int main(int argc, char* argv[]) {
    // Initialize SDL and OpenGL
    initSDL();
    initTexture();

    // Main loop
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Render to SDL texture using OpenGL
        SDL_Delay(10);
        SDL_SetRenderTarget(renderer, sdlTexture);
        renderToTexture();
        SDL_GL_SwapWindow(window);
		SDL_SetRenderTarget(renderer, NULL);
        // Optionally add a delay or frame limiting logic here

        // Swap the front and back buffers
        
    }

    // Clean up
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
