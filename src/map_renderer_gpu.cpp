#include "Game.h"
#include "textre_drawer.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;

Game game = Game();

class Atlas{
    GLuint texture_atlas;


    public:
    vec2 atlas_size = {0, 0};
    ivec3 tile_size;
    ivec3 tile_offsets;

    vec4 get_tile_rect(int i){
        return vec4{i * (tile_size.x + tile_size.z), 0, tile_size.x + tile_size.z, tile_size.y + tile_size.z};
    }

    Atlas(const char* path, ivec3 tile_size, ivec3 tile_offsets){
        SDL_Surface *surface = IMG_Load(path);
        glGenTextures(1, &texture_atlas);
        glBindTexture(GL_TEXTURE_2D, texture_atlas);
        game.set_default_image_settings();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
        atlas_size = {surface->w, surface->h};
        SDL_FreeSurface(surface);
        glBindTexture(GL_TEXTURE_2D, 0);
        this->tile_size = tile_size;
        this->tile_offsets = tile_offsets;
    }

    void from_atlas_to_screen(vec4 src, vec4 dst, vec2 src_size, vec2 dst_size){
        // x y w h -> x0 y0 x1 y1
        src.z += src.x;
        src.w += src.y;
        dst.z += dst.x;
        dst.w += dst.y;

        vec4 uv_src = {
            remap(src.x, 0, src_size.x, 0, 1),
            remap(src.y, 0, src_size.y, 0, 1),
            remap(src.z, 0, src_size.x, 0, 1),
            remap(src.w, 0, src_size.y, 0, 1),
        };
        vec4 uv_dst = {
            remap(dst.x, 0, dst_size.x, -1, 1),
            remap(dst.y, 0, dst_size.y, 1, -1),
            remap(dst.z, 0, dst_size.x, -1, 1),
            remap(dst.w, 0, dst_size.y, 1, -1),
        };
        glVertexAttrib2f(1, uv_src.x, uv_src.y); glVertex2f(uv_dst.x, uv_dst.y);
        glVertexAttrib2f(1, uv_src.z, uv_src.y); glVertex2f(uv_dst.z, uv_dst.y);
        glVertexAttrib2f(1, uv_src.z, uv_src.w); glVertex2f(uv_dst.z, uv_dst.w);
        glVertexAttrib2f(1, uv_src.x, uv_src.w); glVertex2f(uv_dst.x, uv_dst.w);
    }

    void draw_tile(vec2 dst_size, int i, ivec3 dest, GLuint shader){

        dest *= (tile_size - tile_offsets);
        vec4 src_rect = get_tile_rect(i);
        vec4 dest_rect = {1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.z, src_rect.w};
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_atlas);
        glBegin(GL_QUADS);
        from_atlas_to_screen(src_rect, dest_rect, atlas_size, dst_size);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    GLuint get_atlas(){
        return texture_atlas;
    }

    ~Atlas(){
        glDeleteTextures(1, &texture_atlas);
    }
};



//*     Y
//*     |
//*     * -- X
//*      \
//*        Z




class Map{ 
    GLuint map_texture;
    GLuint map_depth;
    GLuint map_normalmap;
    GLuint buffer;

    GLuint tile_shader;

    const char* tile_shader_vert_src = R"(
        #version 330 core
        in vec2 in_tex;
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;

        out vec2 fragTexCoord;

        void main() {
            fragTexCoord = texCoord;
            gl_Position = vec4(position, 1.);
        }
    )";

    const char* tile_shader_frag_src = R"(
        #version 330 core
        in vec2 fragTexCoord;
        out vec4 map_texture;
        out vec4 map_depth;
        out vec4 map_normalmap;

        uniform sampler2D texture_atlas;

        uniform int neighbors;

        uniform vec2 texture_size;
        uniform vec2 tile_size;

        uniform vec2 tile_depth_range;
        
        void main() {
            //gl_FragCoord / texture_size
            vec2 pixel = 1 / texture_size;

            map_texture = texture(texture_atlas, fragTexCoord);
            /*if (frag0.a == 0){
                frag0 = texture(texture_atlas, fragTexCoord + (tile_size * vec2(0., 1.))*pixel);
            }*/

            map_depth = texture(texture_atlas, fragTexCoord + (tile_size * vec2(0., 1.)*pixel*2.));
            map_depth.rgb *= (tile_depth_range.y - tile_depth_range.x);
            map_depth.rgb += tile_depth_range.x;

            map_texture.rgb *= 0.5 + (map_depth.rgb) * 0.5;

            map_normalmap = texture(texture_atlas, fragTexCoord + (tile_size * vec2(0., 1.)*pixel*3.));
        }
    )";

    Atlas *atlas;
    vector<vector<vector<int>>> data;
    ivec3 map_size;
    public:
    ivec2 texture_size;
    ivec2 offset = ivec2();
    Map(vector<vector<vector<int>>> new_data, Atlas* atlas){
        data = new_data;
        this->atlas = atlas;
        ivec3 tile_size = this->atlas->tile_size;
        map_size = ivec3(data[0][0].size(), data.size(), data[0].size());
        texture_size = ivec2(tile_size.x * map_size.x + tile_size.z * map_size.z + 2, tile_size.z * map_size.z + tile_size.y * map_size.y + 2);

        glGenTextures(1, &map_texture);
        glBindTexture(GL_TEXTURE_2D, map_texture);
        game.set_default_image_settings();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.x, texture_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenTextures(1, &map_normalmap);
        glBindTexture(GL_TEXTURE_2D, map_normalmap);
        game.set_default_image_settings();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.x, texture_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenTextures(1, &map_depth);
        glBindTexture(GL_TEXTURE_2D, map_depth);
        game.set_default_image_settings();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.x, texture_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenFramebuffers(1, &buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, map_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, map_depth, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, map_normalmap, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GLint success;

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &tile_shader_vert_src, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
         if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "Tile shader compilation failed: " << infoLog << std::endl;
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &tile_shader_frag_src, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "Tile shader compilation failed: " << infoLog << std::endl;
        }


        tile_shader = glCreateProgram();
        glAttachShader(tile_shader, vertexShader);
        glAttachShader(tile_shader, fragmentShader);
        glLinkProgram(tile_shader);
        glGetProgramiv(tile_shader, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(tile_shader, 512, NULL, infoLog);
            std::cerr << "Tile shader program linking failed: " << infoLog << std::endl;
        }
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
    }
    ivec2 get_texture_size(){return texture_size;}
    ivec3 get_map_size(){return map_size;}
    GLuint get_texture(){return map_texture;}
    GLuint get_depth(){return map_depth;}
    GLuint get_normalmap(){return map_normalmap;}
    void draw(vec2 dst_size){
        vec4 dst = {offset.x, offset.y, offset.x + texture_size.x, offset.y + texture_size.y};
        vec4 dst_uv = {
            remap(dst.x, 0, dst_size.x, -1, 1),
            remap(dst.y, 0, dst_size.y, -1, 1),
            remap(dst.z, 0, dst_size.x, -1, 1),
            remap(dst.w, 0, dst_size.y, -1, 1),
        };
        glBindTexture(GL_TEXTURE_2D, map_texture);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(dst_uv.x, dst_uv.y);
        glTexCoord2f(1, 0); glVertex2f(dst_uv.z, dst_uv.y);
        glTexCoord2f(1, 1); glVertex2f(dst_uv.z, dst_uv.w);
        glTexCoord2f(0, 1); glVertex2f(dst_uv.x, dst_uv.w);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool has_tile_wrapped(ivec3 pos){
        if (pos.x < 0 || pos.y < 0 || pos.z < 0) return false;
        if (pos.y >= data.size() || pos.z >= data[0].size() || pos.x >= data[0][0].size()) return false;
        return data[pos.y][pos.z][pos.x] != 0;
    }

    int get_tile_neighbors_state(ivec3 pos){
        /*

        1   -X
        2   -Y
        4   -Z
        8   +X
        16  +Y
        32  +Z

        */
        int state = 0;
        if (has_tile_wrapped(pos + ivec3(-1.,  0.,  0.))) state += 1;
        if (has_tile_wrapped(pos + ivec3( 0., -1.,  0.))) state += 2;
        if (has_tile_wrapped(pos + ivec3( 0.,  0., -1.))) state += 4;
        if (has_tile_wrapped(pos + ivec3( 1.,  0.,  0.))) state += 8;
        if (has_tile_wrapped(pos + ivec3( 0.,  1.,  0.))) state += 16;
        if (has_tile_wrapped(pos + ivec3( 0.,  0.,  1.))) state += 32;
        return state;
    }

    // ! warn ! loses fbo context!
    void render(Atlas* atlas){
        /*
        outdated....
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        gl_FragData in shader
        */
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        GLenum attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);  
        glViewport(0, 0, texture_size.x, texture_size.y);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_BLEND_COLOR);
        glUseProgram(tile_shader);
        vec4 tile_rect_size = atlas->get_tile_rect(0);

        

        glUniform1i(glGetUniformLocation(tile_shader, "texture_atlas"), 0);
        glUniform2f(glGetUniformLocation(tile_shader, "texture_size"), atlas->atlas_size.x, atlas->atlas_size.y);
        glUniform2f(glGetUniformLocation(tile_shader, "tile_size"), tile_rect_size.z, tile_rect_size.w);

        float depth_step = 1. / (float)(map_size.z+1);

        for (int y = 0; y < map_size.y; y++){
            for (int z = 0; z < map_size.z; z++){
                float tile_depth = depth_step * z;
                glUniform2f(glGetUniformLocation(tile_shader, "tile_depth_range"), tile_depth, tile_depth+depth_step);
                for (int x = map_size.x - 1; x >= 0; x--){
                    int tile = data[y][z][x];
                    if (tile != 0){
                        vec4 rect = atlas->get_tile_rect(tile - 1);
                        int neighbors_state = get_tile_neighbors_state({x, y, z});
                        glUniform1i(glGetUniformLocation(tile_shader, "neighbors"), neighbors_state);
                        atlas->draw_tile(get_texture_size(), tile - 1, ivec3(x, y - map_size.y + 1, z), tile_shader);
                    }
                }
            }
        }
        glUseProgram(0);
        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~Map(){ // cleanup
        if (map_texture != 0) glDeleteTextures(1, &map_texture);
        if (map_normalmap != 0) glDeleteTextures(1, &map_normalmap);
        if (map_depth != 0) glDeleteTextures(1, &map_depth);
        glDeleteFramebuffers(1, &buffer);
    }
};



/*
    passes:
        sky
        draw map (apply normalmap lightning) maybe weather?
        ??? draw map lightning based on physics ???
        draw entities
        draw particles
        ??? reflections ???
*/


int main(int argc, char ** argv)
{
    game.init();
    
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

    int TEX_SIZE = 64;
    TextureDrawer drawer = TextureDrawer(TEX_SIZE, TEX_SIZE, TEX_SIZE);
    drawer.fill(DRAWER_WHITE);
    for (int z = 0; z < TEX_SIZE; z++)
    for (int y = 0; y < TEX_SIZE; y++)
    for (int x = 0; x < TEX_SIZE; x++)
    drawer.set_pixel(x, y, z, x > TEX_SIZE * 0.5 ? 0. : 255., y > TEX_SIZE * 0.5 ? 0. : 255., z > TEX_SIZE * 0.5 ? 0. : 255.);
    GLubyte* character_texture_data = drawer.get_data();
    SDF_Frag_Shader shader = SDF_Frag_Shader("assets/shaders/sdf_scene.frag", "assets/shaders/sdf_scene.vert", &game.debugger);
    vec2 shader_texture_size = ivec2(48, 48);
    shader_texture_size = ivec2(128, 128);
    shader.init(shader_texture_size.x, shader_texture_size.y, ivec3(TEX_SIZE), character_texture_data);
    drawer.destroy();
    ObjectScene scene;
    PrimitiveScene primitive_scene;

    BoxObject* box = new BoxObject(vec3(0., 0., 0.), vec3(1., 1., 1.));
    scene.objects.push_back(box);
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

    map.render(&atlas);
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

        glUseProgram(shaderProgram);
        glUniform2f(glGetUniformLocation(shaderProgram, "target_size"), TARGET_WIDTH, TARGET_HEIGHT);
        glUniform2f(glGetUniformLocation(shaderProgram, "size"), game.get_screen_size().x, game.get_screen_size().y);
        game.draw_fullscreen_quad();
        glUseProgram(0);

        map.offset = (game.screen_pixel_size - map.get_texture_size());
        map.offset /= 2;
        glEnable(GL_BLEND);
        glDisable(GL_BLEND);
        map.draw(game.screen_pixel_size);
        game.end_main();
        game.begin_main();
        shader.check_file_updates();
        shader.use();
        // x+
        shader.set_1f("time", time);
        shader.set_2f("texture_size", shader_texture_size.x, shader_texture_size.y);
        scene.update_primitive_scene(&primitive_scene);
        shader.set_scene(&primitive_scene);
        shader.set_position({cos(time) * 4 + 4, sin(time * 2) * 2 + 2, sin(time) * 4 + 4});
        shader.set_position({2, 0, 2});
        shader.update_map(map.get_depth(), map.get_texture_size(), map.get_map_size());
        glEnable(GL_BLEND);

        shader.draw(game.screen_pixel_size);
        glDisable(GL_BLEND);
        game.end_main();

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        game.draw_main();
        
        SDL_GL_SwapWindow(window);

        // todo: alpha checks for depth buffer draw :D
    }
    shader.destroy();
    glDeleteProgram(shaderProgram);
    IMG_Quit();
    game.destroy();
    return 0;
}