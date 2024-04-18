#include "Game.h"
#include "textre_drawer.h"

using namespace glm;

SDL_Event event;
Game game = Game();
int main(int argc, char ** argv)
{
    game.init();
    
    IMG_Init(IMG_INIT_PNG);

    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 vertexPosition;
        layout(location = 1) in vec4 vertexColor;

        out vec4 fragmentColor;

        void main() {
            gl_Position = vec4(vertexPosition, 1.0);
            fragmentColor = vertexColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core

        in vec4 fragmentColor;
        out vec4 FragColor;

        uniform vec2 size;

        void main() {
            vec2 pixelPos = gl_FragCoord.xy;
            
            FragColor = ((int(pixelPos.x + pixelPos.y) % 2) == 0) ? fragmentColor : vec4(fragmentColor.rgb * 0.5, 1.);
            vec2 big = (size + vec2(320, 240)) / 2.;
            vec2 sml = (size - vec2(320, 240)) / 2.;
            if (pixelPos.x < big.x && pixelPos.x > sml.x && pixelPos.y < big.y && pixelPos.y > sml.y) FragColor.b = 1.;
        }
    )";

    GLint success;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
    }



    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
    }



    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);


    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
    }


    while (game.is_running())
    {
        float time = game.time();
        while (SDL_PollEvent(&event))
        {
            game.handle_event(event);
        }
        
        game.begin_main();
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform2f(glGetUniformLocation(shaderProgram, "size"), game.get_screen_size().x, game.get_screen_size().y);
        glUseProgram(shaderProgram);
        glUniform2f(glGetUniformLocation(shaderProgram, "size"), game.get_screen_size().x, game.get_screen_size().y);

        glBegin(GL_QUADS);
        glVertexAttrib4f(1, 1, 0, 0, 1);
        glVertex3f(-1, 1, 0);
        glVertexAttrib4f(1, 1, 1, 0, 1);
        glVertex3f(1, 1, 0);
        glVertexAttrib4f(1, 0, 1, 0, 1);
        glVertex3f(1, -1, 0);
        glVertexAttrib4f(1, 0, 0, 0, 1);
        glVertex3f(-1, -1, 0);
        glEnd();


        glUseProgram(0);

        game.end_main();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game.draw_main();
        SDL_GL_SwapWindow(window);
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    IMG_Quit();
    game.destroy();
    return 0;
}