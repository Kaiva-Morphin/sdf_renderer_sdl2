#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <vector>
using namespace std;
const char* computeShaderSource = R"(
    #version 430 core

    struct Primitive{
        int type;
    };

    layout(std430, binding = 0) buffer InBuffer {
        Primitive primitives[];
    };

    layout(std430, binding = 1) buffer OutBuffer {
        Primitive outputPrimitives[];
    };

    layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    
    void main() {
        uint index = gl_GlobalInvocationID.x;
        outputPrimitives[index] = primitives[index];
    }
)";

struct Primitive{
    int type;
};

struct Scene{
    std::vector<Primitive> data;
};


int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Simple Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    Scene scene;
    scene.data.push_back(Primitive{2});
    scene.data.push_back(Primitive{4});
    scene.data.push_back(Primitive{8});
    scene.data.push_back(Primitive{1});

    GLuint inBuffer, outBuffer;
    glGenBuffers(1, &inBuffer);

    glGenBuffers(1, &outBuffer);

    // Load and compile the compute shader
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
    glCompileShader(computeShader);
    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
        return -1;
    }
    // Create the compute shader program
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(computeProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return -1;
    }
    glUseProgram(computeProgram);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, scene.data.size() * sizeof(Primitive), &scene.data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, scene.data.size() * sizeof(Primitive), nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outBuffer);

    glDispatchCompute(scene.data.size(), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    std::vector<Primitive> outputData(scene.data.size());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, scene.data.size() * sizeof(Primitive), &outputData[0]);
    for (auto a: outputData) cout << a.type << endl;
    // Cleanup
    glDeleteProgram(computeProgram);
    glDeleteShader(computeShader);
    glDeleteBuffers(1, &inBuffer);
    glDeleteBuffers(1, &outBuffer);

    // Terminate GLFW
    // Cleanup GLFW resources

    return 0;
}
