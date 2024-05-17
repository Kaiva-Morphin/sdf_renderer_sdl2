#include "Game.h"
#include "BdfFont.h"
#include "textre_drawer.h"
#include "Physics.h"
#include "Map.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;





int main(int argc, char ** argv)
{
    game = new Game();
    game->init();
    
    IMG_Init(IMG_INIT_PNG);


    vector<vector<vector<int>>> map_data = {
        {
            {0, 1, 1, 1, 1, 1, 1, 1},
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




    map_data = {
        {
            {1, 0, 0, 2, 0, 0, 3, 0, 0, 4, 0, 0, 5, 0, 0}
        }
    };

    PhysicsSolver solver("");

    
        

    // x z y
    ivec3 tile_size = ivec3(17, 11, 9);
    ivec3 tile_offsets = ivec3(1, 1, 1);
    Atlas atlas = Atlas("assets/better_tiles.png", tile_size, tile_offsets);

    //atlas.generate_state_table();

    //return 0;

    // 0 1 2 3 4
    // 1 2 4


    Map map = Map(map_data, &atlas);

    map.render(&atlas);

    vec3 map_size = map.get_map_size();
    for (int y = 0; y < map_size.y; y++){
        for (int z = 0; z < map_size.z; z++){
            for (int x = map_size.x - 1; x >= 0; x--){
                int tile = map_data[y][z][x];
                //int n = map.get_tile_neighbors_state({x, y, z});
                if (tile == 1){

                    //*     Y
                    //*     |
                    //*     * -- X
                    //*      \
                    //*        Z

                    vec3 pos = vec3(x, y, z);
                    vec3 half_size = vec3(0.25, 0.25, 0.25);
                    vec3 right_up_near =   half_size * vec3( 1,  1,  1);
                    vec3 right_up_far =    half_size * vec3( 1,  1, -1);
                    vec3 right_down_near = half_size * vec3( 1, -1,  1);
                    vec3 right_down_far =  half_size * vec3( 1, -1, -1);
                    vec3 left_up_near =    half_size * vec3(-1,  1,  1);
                    vec3 left_up_far =     half_size * vec3(-1,  1, -1);
                    vec3 left_down_near =  half_size * vec3(-1, -1,  1);
                    vec3 left_down_far =   half_size * vec3(-1, -1, -1);


                    PhysicsPrimitive *up1 = new PhysicsPrimitive;
                    *up1 = solver.triangle(pos, right_up_near, right_up_far, left_up_near);
                    solver.push(up1);
                    PhysicsPrimitive *up2 = new PhysicsPrimitive;
                    *up2 = solver.triangle(pos, left_up_far, left_up_near, right_up_far);
                    solver.push(up2);

                    PhysicsPrimitive *down1 = new PhysicsPrimitive;
                    *down1 = solver.triangle(pos, right_down_near, left_down_near, right_down_far);
                    solver.push(down1);
                    PhysicsPrimitive *down2 = new PhysicsPrimitive;
                    *down2 = solver.triangle(pos, left_down_far, right_down_far, left_down_near);
                    solver.push(down2);

                    PhysicsPrimitive *front1 = new PhysicsPrimitive;
                    *front1 = solver.triangle(pos, right_up_near, left_up_near, right_down_near);
                    solver.push(front1);
                    PhysicsPrimitive *front2 = new PhysicsPrimitive;
                    *front2 = solver.triangle(pos, left_down_near, right_down_near, left_up_near);
                    solver.push(front2);

                    PhysicsPrimitive *back1 = new PhysicsPrimitive;
                    *back1 = solver.triangle(pos, right_up_far, right_down_far, left_up_far);
                    solver.push(back1);
                    PhysicsPrimitive *back2 = new PhysicsPrimitive;
                    *back2 = solver.triangle(pos, left_down_far, left_up_far, right_down_far);
                    solver.push(back2);

                    PhysicsPrimitive *left1 = new PhysicsPrimitive;
                    *left1 = solver.triangle(pos, left_down_near, left_up_near, left_down_far);
                    solver.push(left1);
                    PhysicsPrimitive *left2 = new PhysicsPrimitive;
                    *left2 = solver.triangle(pos, left_up_far, left_down_far, left_up_near);
                    solver.push(left2);

                    PhysicsPrimitive *right1 = new PhysicsPrimitive;
                    *right1 = solver.triangle(pos, right_down_near, right_down_far, right_up_near);
                    solver.push(right1);
                    PhysicsPrimitive *right2 = new PhysicsPrimitive;
                    *right2 = solver.triangle(pos, right_up_far, right_up_near, right_down_far);
                    solver.push(right2);

                }
            }
        }
    }

    int TEX_SIZE = 64;
    TextureDrawer drawer = TextureDrawer(TEX_SIZE, TEX_SIZE, TEX_SIZE);
    drawer.fill(DRAWER_WHITE);
    for (int z = 0; z < TEX_SIZE; z++)
    for (int y = 0; y < TEX_SIZE; y++)
    for (int x = 0; x < TEX_SIZE; x++)
    drawer.set_pixel(x, y, z, x > TEX_SIZE * 0.5 ? 0. : 255., y > TEX_SIZE * 0.5 ? 0. : 255., z > TEX_SIZE * 0.5 ? 0. : 255.);
    GLubyte* character_texture_data = drawer.get_data();
    SDF_Frag_Shader shader = SDF_Frag_Shader("assets/shaders/sdf_scene.frag");
    vec2 shader_texture_size = ivec2(48, 48);
    shader_texture_size = ivec2(128, 128);
    shader.init(shader_texture_size.x, shader_texture_size.y, ivec3(TEX_SIZE), character_texture_data);
    drawer.destroy();
    ObjectScene scene;
    PrimitiveScene primitive_scene;

    //BoxObject* box = new BoxObject(vec3(0., 0., 0.), vec3(1., 2., 1.));
    SphereObject* sphere = new SphereObject(1);
    scene.objects.push_back(sphere);
    scene.ordered_operations.push_back(
        PrimitiveOperation{
            OPERATION_UNION,
            -1,
            0,
            0,
            1
        }
    );


    scene.update_primitive_scene(&primitive_scene);

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 frag;
        uniform vec2 size;
        uniform vec2 target_size;
        
        mat4 bayerIndex = mat4(
            vec4(00.0/16.0, 12.0/16.0, 03.0/16.0, 15.0/16.0),
            vec4(08.0/16.0, 04.0/16.0, 11.0/16.0, 07.0/16.0),
            vec4(02.0/16.0, 14.0/16.0, 01.0/16.0, 13.0/16.0),
            vec4(10.0/16.0, 06.0/16.0, 09.0/16.0, 05.0/16.0));
        
        float dither8x8(float r) {    
            float threshold = bayerIndex[int(gl_FragCoord.x)%4][int(gl_FragCoord.y)%4];
            //return mix(r, r * floor(floor(r * 64.) / 64. + threshold) * 1.5, 1);
            return r > threshold ? 1:0;
        }

        void main() {
            vec3 color1 = vec3(0.7, 0.94, 0.99);
            color1 = vec3(0.102, 0.6, 0.713) * 0.7;
            vec3 color2 = vec3(0.04, 0.35, 0.45) * 0.7;
            vec2 uv = gl_FragCoord.xy / size.xy;
            float y = clamp((gl_FragCoord.y - size.y / 4.) / 32 - cos((gl_FragCoord.x-size.x/2.)/size.x * 4.) * 0.5, 0, 1);
            frag = vec4(mix(color1, color2, dither8x8(y)), 1.);

            //if (pixelPos.x < big.x && pixelPos.x > sml.x && pixelPos.y < big.y && pixelPos.y > sml.y) FragColor.b = 1.;

            //frag = pixelPos.y > target_size.y / 2 ? vec4(0.7, 0.94, 0.99, 1.) : vec4(0.04, 0.35, 0.45, 1.);
        }
    )";

    GLint success;

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
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
    }
    
    BDFAtlas font_atlas = BDFAtlas("assets/fonts/orp/orp-italic.bdf", 1536);
    game->debugger.init(&font_atlas);
    
    PhysicsPrimitive player = solver.capsule(0.001, sphere->rounding);
    player.type = TYPE_RIGID;
    player.bounciness = 0.00001;
    player.mass = 1.;
    player.position.x = 3;
    player.position.y = 0;
    player.position.z = 3;
    player.friction = 0.5;
    solver.push(&player);

    bool key_a, key_d, key_w, key_s, key_space, key_shift;
    key_a = false;
    key_d = false;
    key_w = false;
    key_s = false;
    key_space = false;
    key_shift = false;

    game->debugger.register_line("vel","velocity: ","0, 0");
    game->debugger.register_line("inp","inp: ","0, 0");
    game->debugger.register_line("pos", "pos:  ", "0 0 0");
    game->debugger.register_line("phys", "collide:  ", "");
    game->debugger.register_line("phys0", "normal:  ", "");
    game->debugger.register_line("phys1", "phys:  ", "");
    game->debugger.register_line("phys2", "phys:  ", "");

    map.render(&atlas);
    while (game->is_running())
    {
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



        vec2 target_vel = vec2(key_d - key_a,  key_s - key_w) * 4.0f;
        //cout << player.velocity.z << endl;
        player.velocity.z = target_vel.y;
        player.velocity.x = target_vel.x;
        player.velocity.y = (key_space - key_shift) * 4.0f;


        game->debugger.update_line("vel", to_string(player.velocity.x) + " " + to_string(player.velocity.y) + " " + to_string(player.velocity.z));
        game->debugger.update_line("inp", to_string(target_vel.x) + " " + to_string(target_vel.y));
        game->debugger.update_line("pos", to_string(player.position.x) + " " + to_string(player.position.y) + " " + to_string(player.position.z));

        game->begin_main();
        
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform2f(glGetUniformLocation(shaderProgram, "target_size"), TARGET_WIDTH, TARGET_HEIGHT);
        glUniform2f(glGetUniformLocation(shaderProgram, "size"), game->get_screen_size().x, game->get_screen_size().y);
        game->draw_fullscreen_quad();
        glUseProgram(0);

        map.offset = (game->screen_pixel_size - map.get_texture_size());
        map.offset /= 2;
        glEnable(GL_BLEND);
        map.draw(game->screen_pixel_size);
        glDisable(GL_BLEND);
        game->end_main();

        game->begin_main();
        shader.check_file_updates();
        shader.use();
        // x+
        shader.set_1f("time", time);
        shader.set_2f("texture_size", shader_texture_size.x, shader_texture_size.y);
        scene.update_primitive_scene(&primitive_scene);
        shader.set_scene(&primitive_scene);
        solver.step(game->wrapped_delta(), nullptr);

        shader.set_position(player.position);

        //shader.set_position({0, 0, 0});

        shader.update_map(map.get_depth(), map.get_texture_size(), map.get_map_size());
        
        glEnable(GL_BLEND);
        shader.draw(game->screen_pixel_size);
        glDisable(GL_BLEND);

        //solver.draw_ZY(10);

        game->debugger.update_basic();
        game->debugger.draw(game->screen_pixel_size);

        game->end_main();

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game->draw_main();

        SDL_GL_SwapWindow(window);
        // todo: alpha checks for depth buffer draw :D
    }
    shader.destroy();
    glDeleteProgram(shaderProgram);
    IMG_Quit();
    game->destroy();
    return 0;
}