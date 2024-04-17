#include "header.h"
#include "sdf_primitives.h"
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
    public:
    ivec3 tile_size;
    SDL_Rect get_tile_rect(int i){
        return SDL_Rect{i * (tile_size.x + tile_size.z), 0, tile_size.x + tile_size.z, tile_size.y + tile_size.z};
    }
    Atlas(const char* path, ivec3 tile_size){
        SDL_Surface *surface = IMG_Load(path);
        texture_atlas = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        this->tile_size = tile_size;
    }
    void set_depth(int d){
        SDL_SetTextureColorMod(texture_atlas, d, d, d);
    }
    void set_alpha(int a){
        SDL_SetTextureAlphaMod(texture_atlas, a);
    }
    void draw_depth_tile(int i, ivec3 dest){
        dest *= tile_size;
        SDL_Rect src_rect = get_tile_rect(i);
        src_rect.y += (tile_size.y + tile_size.z) * 3.;
        SDL_Rect dest_rect = SDL_Rect{1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.w, src_rect.h};
        SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);
    }
    void draw_tile_depth(int i, ivec3 dest){
        dest *= tile_size;
        SDL_Rect src_rect = get_tile_rect(i);
        src_rect.y += (tile_size.y + tile_size.z) * 2.;
        SDL_Rect dest_rect = SDL_Rect{1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.w, src_rect.h};
        SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);
    }
    void draw_tile(int i, ivec3 dest){
        dest *= tile_size;
        SDL_Rect src_rect = get_tile_rect(i);
        SDL_Rect dest_rect = SDL_Rect{1 + dest.x + dest.z, 1 + dest.z - dest.y, src_rect.w, src_rect.h};
        SDL_RenderCopy(renderer, texture_atlas, &src_rect, &dest_rect);
    }
    void draw_tile_normals(int i, ivec3 dest){
        dest *= tile_size;
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
    // todo: caching map's colors, normals, depth, ?materials? to gl buffers
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

    void render(Atlas* atlas){
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_SetRenderTarget(renderer, depth_texture);
        SDL_RenderClear(renderer);
        float depth_step = 1. / (float)(size.z+1) * 255.;
        for (int y = 0; y < size.y; y++){
            for (int z = 0; z < size.z; z++){
                float depth = (float)(z+1) / (float)(size.z+1) * 255.;
                for (int x = size.x - 1; x >= 0; x--){
                    int tile = data[y][z][x];
                    if (tile != 0){

                        atlas->set_depth(depth);
                        atlas->set_alpha(255);
                        atlas->draw_tile_depth(tile - 1, ivec3(x, y - size.y + 1, z));

                        atlas->set_depth(255);
                        atlas->set_alpha(depth_step);
                        atlas->draw_depth_tile(tile - 1, ivec3(x, y - size.y + 1, z));
                        
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
                        atlas->draw_tile(tile - 1, ivec3(x, y - size.y + 1, z));
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

Game game = Game();
int main(int argc, char ** argv)
{
    game.init();
    game.debugger.register_basic();

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
    ivec3 tile_size = ivec3(24, 12, 8);
    Atlas atlas = Atlas("assets/tiles.png", tile_size);
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
    SDF_Shader shader = SDF_Shader("assets/shader.glsl", &game.debugger);
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
        float time = SDL_GetTicks() / 1000.0f;
        while (SDL_PollEvent(&event))
        {
            game.handle_event(event);
        }
        SDL_Delay(10);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        map.render(&atlas);
        game.switch_to_main();
        SDL_SetRenderDrawColor(renderer, 20, 20, 100, 255);
        SDL_Rect rect = SDL_Rect{0, 0, TARGET_WIDTH, TARGET_HEIGHT};
        SDL_RenderFillRect(renderer, &rect);
        ivec2 map_size = map.get_texture_size();
        //map.offset = ivec3((float)(TARGET_WIDTH - map_size.x) / 2., (float)(TARGET_HEIGHT - map_size.y) / 2., 0);
        map.draw();
        shader.check_file_updates();
        shader.use();
        shader.set_1f("time", time);
        shader.set_2f("center", shader_texture_size.x / 2., shader_texture_size.y / 2.);
        scene.update_primitive_scene(&primitive_scene);
        shader.set_scene(&primitive_scene);
        shader.run();
        shader.wait();
        rect = {20, 20, shader_texture_size.x, shader_texture_size.y};
        SDL_RenderCopy(renderer, shader.get_texture(), nullptr, &rect);

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