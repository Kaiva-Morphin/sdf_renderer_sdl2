#include "Game.h"
#include "textre_drawer.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;

void init(){
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Map Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
    SDL_SetWindowResizable(window, SDL_bool::SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, TARGET_WIDTH, TARGET_HEIGHT);
    //SDL_RenderSetIntegerScale(renderer, SDL_bool::SDL_TRUE); //for true pixel perfect
}
void cleanup(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
class Atlas{
    SDL_Texture* texture_atlas;
    ivec4 state_table[1][8]; // state -> [rects]
    public:
    ivec3 tile_size;
    ivec3 tile_offsets;
    SDL_Rect get_tile_rect(int i){
        return SDL_Rect{i * (tile_size.x + tile_size.z), 0, tile_size.x + tile_size.z, tile_size.y + tile_size.z};
    }
    Atlas(const char* path, ivec3 tile_size, ivec3 tile_offsets){
        SDL_Surface *surface = IMG_Load(path);
        texture_atlas = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        this->tile_size = tile_size;
        this->tile_offsets = tile_offsets;
    }

    void generate_state_table(){
        SDL_Rect tile_rect = get_tile_rect(0);
        ivec4 tile = {0, 0, tile_rect.w, tile_rect.h};
    }

    void set_depth(int d){
        SDL_SetTextureColorMod(texture_atlas, d, d, d);
    }
    void set_alpha(int a){
        SDL_SetTextureAlphaMod(texture_atlas, a);
    }
    void draw_depth_tile(int i, ivec3 dest){
        dest *= (tile_size - tile_offsets);
        SDL_Rect src_rect = get_tile_rect(i);
        src_rect.y += (tile_size.y + tile_size.z) * 4.;
        SDL_Rect dest_rect = SDL_Rect{1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.w, src_rect.h};
        SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);
    }
    void draw_tile_depth(int i, ivec3 dest){
        dest *= (tile_size - tile_offsets);
        SDL_Rect src_rect = get_tile_rect(i);
        src_rect.y += (tile_size.y + tile_size.z) * 3.;
        SDL_Rect dest_rect = SDL_Rect{1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.w, src_rect.h};
        SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);
    }

    void draw_tile(int i, ivec3 dest, int state){ // todo: rewrite for gpu!
        dest *= (tile_size - tile_offsets);
        SDL_Rect src_rect = get_tile_rect(i);
        SDL_Rect dest_rect = SDL_Rect{1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.w, src_rect.h};
        SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);

        src_rect.y += tile_size.y + tile_size.z;
        // minimize draw calls!
        /*if (state == 0){ 
            SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);
            return;
        }*/
        /*

        n   a    d
        0   1   -X 
        1+  2   -Y 
        2+  4   -Z 
        3+  8   +X 
        4   16  +Y 
        5   32  +Z 

        */

        if ((state & (1 << 2)) == 0){ // -Z
            SDL_Rect src_rect_temp = src_rect;
            src_rect_temp.w = 18;
            src_rect_temp.h = 11;
            SDL_Rect dest_rect_temp = dest_rect;
            dest_rect_temp.w = 18;
            dest_rect_temp.h = 11;
            SDL_RenderCopy(renderer, texture_atlas, &src_rect_temp, &dest_rect_temp);
        }
        if ((state & (1 << 1)) == 0){ // -Y
            SDL_Rect src_rect_temp = src_rect;
            src_rect_temp.y += 11;
            src_rect_temp.w = 25;
            src_rect_temp.h = 9;
            SDL_Rect dest_rect_temp = dest_rect;
            dest_rect_temp.y += 11;
            dest_rect_temp.w = 25;
            dest_rect_temp.h = 9;
            if ((state & (1 << 0)) != 0) { // -X // FIX
                src_rect_temp.x += 9;
                src_rect_temp.y += 7;
                src_rect_temp.h = 2;
                src_rect_temp.w = 16;
                dest_rect_temp.x += 9;
                dest_rect_temp.y += 7;
                dest_rect_temp.h = 2;
                dest_rect_temp.w = 16;
            }
            SDL_RenderCopy(renderer, texture_atlas, &src_rect_temp, &dest_rect_temp);
        }
        if ((state & (1 << 3)) == 0){ // +X
            SDL_Rect src_rect_temp = src_rect;
            src_rect_temp.x += 18;
            src_rect_temp.w = 8;
            src_rect_temp.h = 19;
            SDL_Rect dest_rect_temp = dest_rect;
            dest_rect_temp.x += 18;
            dest_rect_temp.w = 8;
            dest_rect_temp.h = 19;
            SDL_RenderCopy(renderer, texture_atlas, &src_rect_temp, &dest_rect_temp);
        }
    }
    void draw_tile_normals(int i, ivec3 dest){
        dest *= (tile_size - tile_offsets);
        SDL_Rect src_rect = get_tile_rect(i);
        src_rect.y += tile_size.y + tile_size.z;
        SDL_Rect dest_rect = SDL_Rect{1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.w, src_rect.h};
        SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);
    }
    ~Atlas(){
        SDL_DestroyTexture(texture_atlas);
    }
};


//*     Y
//*     |
//*     * -- X
//*      \
//*        Z




class Map{ 
    // todo: caching map to texture and make depthmap for shadows and pure rendering
    SDL_Texture *texture;
    SDL_Texture *normals_texture;
    SDL_Texture *depth_texture;

    ivec2 texture_size;
    Atlas *atlas;
    vector<vector<vector<int>>> data;
    ivec3 size;
    public:
    ivec3 offset = ivec3();
    Map(vector<vector<vector<int>>> new_data, Atlas* atlas){
        data = new_data;
        this->atlas = atlas;
        ivec3 tile_size = this->atlas->tile_size;
        size = ivec3(data[0][0].size(), data.size(), data[0].size());
        texture_size = ivec2(tile_size.x * size.x + tile_size.z * size.z + 2, tile_size.z * size.z + tile_size.y * size.y + 2);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_size.x, texture_size.y);
        depth_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_size.x, texture_size.y);
        normals_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_size.x, texture_size.y);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    ivec2 get_texture_size(){return texture_size;}
    void draw(){
        SDL_Rect dstrect = SDL_Rect{offset.x, offset.y, texture_size.x, texture_size.y};
        SDL_RenderCopy(renderer, texture, nullptr, &dstrect);
    }
    void draw_normals(){
        SDL_Rect dstrect = SDL_Rect{offset.x, offset.y, texture_size.x, texture_size.y};
        SDL_RenderCopy(renderer, normals_texture, nullptr, &dstrect);
    }
    void draw_depth(){
        SDL_Rect dstrect = SDL_Rect{offset.x, offset.y, texture_size.x, texture_size.y};
        SDL_RenderCopy(renderer, depth_texture, nullptr, &dstrect);
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

    void render(Atlas* atlas){
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_SetRenderTarget(renderer, depth_texture);
        SDL_RenderClear(renderer);
        float depth_step = 1. / (float)(size.z+1) * 255.;
        for (int z = 0; z < size.z; z++){
            float depth = (float)(z+1) / (float)(size.z+1) * 255.;
            atlas->set_depth(depth);
            atlas->set_alpha(255);
            for (int y = 0; y < size.y; y++){
                for (int x = size.x - 1; x >= 0; x--){
                    int tile = data[y][z][x];
                    if (tile != 0){
                        atlas->draw_tile_depth(tile - 1, ivec3(x, y - size.y + 1, z));
                        //atlas->set_depth(255);
                        //atlas->set_alpha(depth);
                        //atlas->draw_depth_tile(tile - 1, ivec3(x, y - size.y + 1, z));
                    }
                }
            }
        }
        atlas->set_depth(255);
        atlas->set_alpha(255);
        SDL_SetRenderTarget(renderer, normals_texture);
        SDL_RenderClear(renderer);
        for (int y = 0; y < size.y; y++){
            for (int z = 0; z < size.z; z++){
                for (int x = size.x - 1; x >= 0; x--){
                    int tile = data[y][z][x];
                    if (tile != 0){
                        atlas->draw_tile_normals(tile - 1, ivec3(x, y - size.y + 1, z));
                    }
                }
            }
        }
        SDL_SetRenderTarget(renderer, texture);
        SDL_RenderClear(renderer);
        for (int y = 0; y < size.y; y++){
            for (int z = 0; z < size.z; z++){
                for (int x = size.x - 1; x >= 0; x--){
                    int tile = data[y][z][x];
                    if (tile != 0){
                        atlas->draw_tile(tile - 1, ivec3(x, y - size.y + 1, z), get_tile_neighbors_state(ivec3(x, y, z)));
                    }
                }
            }
        }
        SDL_SetRenderTarget(renderer, NULL);
    }

    ~Map(){ // cleanup
        SDL_DestroyTexture(texture);
    }
};

bool check_quit_event(){
    SDL_PollEvent(&event);
    switch (event.type)
    {
        case SDL_QUIT:
            return true;
    }
    return false;
}

/*
    passes:
        sky
        draw map (apply normalmap lightning) maybe weather?
        ??? draw map lightning based on physics ???
        draw entities
        draw particles
        ??? reflections ???




     

    -> SDL_TEXTURE -> draw map ~> GL_TEXTURE     \
    -> SDL_TEXTURE -> draw normals ~> GL_TEXTURE  } -> shader -> 
    -> SDL_TEXTURE -> draw depth ~> GL_TEXTURE   /


*/



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
    SDF_Shader shader = SDF_Shader("assets/shaders/sdf_scene.comp", &game.debugger);
    ivec2 shader_texture_size = ivec2(48, 48);
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

    

    while (game.is_running())
    {
        float time = game.time();
        while (SDL_PollEvent(&event))
        {
            game.handle_event(event);
        }
        //SDL_Delay(10);
        map.render(&atlas);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        game.switch_to_main();
        
        SDL_SetRenderDrawColor(renderer, 20, 20, 100, 255);
        SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
        SDL_RenderFillRect(renderer, &rect);
        ivec2 map_size = map.get_texture_size();
        //map.offset = ivec3((float)(TARGET_WIDTH - map_size.x) / 2., (float)(TARGET_HEIGHT - map_size.y) / 2., 0);
        map.draw_depth();
        //shader.check_file_updates();
        //shader.use();
        //shader.set_1f("time", time);
        //shader.set_2f("center", shader_texture_size.x / 2., shader_texture_size.y / 2.);
        //scene.update_primitive_scene(&primitive_scene);
        //shader.set_scene(&primitive_scene);
        //shader.run();
        //shader.wait();
        //rect = {20, 20, shader_texture_size.x, shader_texture_size.y};
        //SDL_RenderCopy(renderer, shader.get_texture(), nullptr, &rect);


        // todo: alpha checks for depth buffer draw
        game.debugger.update_basic();
        game.debugger.draw();
        game.apply_main();

        SDL_RenderPresent(renderer);
    }
    IMG_Quit();
    game.destroy();
    cleanup();
    return 0;
}