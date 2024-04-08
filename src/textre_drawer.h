#include <GL/glew.h>
#include <glm/glm.hpp>
using namespace std;

#include <algorithm>
#include <iostream>

#define DRAWER_RED 255, 0, 0
#define DRAWER_GREEN 0, 255, 0
#define DRAWER_BLUE 0, 0, 255
#define DRAWER_WHITE 255, 255, 255
#define DRAWER_BLACK 0, 0, 0
#define DRAWER GRAY 128, 128, 128
#define DRAWER_YELLOW 255, 255, 0
#define DRAWER_CYAN 0, 255, 255
#define DRAWER_MAGENTA 255, 0, 255
#define DRAWER_WHITE 255, 255, 255
#define DRAWER_BLACK 0, 0, 0
#define DRAWER_GRAY 128, 128, 128
#define DRAWER_ORANGE 255, 165, 0
#define DRAWER_PURPLE 128, 0, 128
#define DRAWER_PINK 255, 192, 203
#define DRAWER_BROWN 165, 42, 42
#define DRAWER_LIME 0, 255, 0
#define DRAWER_TEAL 0, 128, 128
#define DRAWER_NAVY 0, 0, 128
#define DRAWER_MAROON 128, 0, 0
#define DRAWER_OLIVE 128, 128, 0
#define DRAWER_SILVER 192, 192, 192
#define DRAWER_GOLD 255, 215, 0
#define DRAWER_SKY_BLUE 135, 206, 235
#define DRAWER_INDIGO 75, 0, 130
#define DRAWER_TURQUOISE 64, 224, 208
#define DRAWER_SALMON 250, 128, 114
#define DRAWER_CORAL 255, 127, 80
#define DRAWER_IVORY 255, 255, 240
#define DRAWER_LAVENDER 230, 230, 250
#define DRAWER_SEA_GREEN 46, 139, 87

class TextureDrawer{
    int w, h, d;
    int r, g, b;
    public:
    GLubyte* data;
    TextureDrawer(int width, int height, int depth){
        w = width;
        h = height;
        d = depth;
        r = 0.;
        g = 0.;
        b = 0.;
        data = new GLubyte[w * h * d * 3];
    }

    void fill(int r, int g, int b){
        for (int x = 0; x < w; ++x)
        for (int y = 0; y < h; ++y)
        for (int z = 0; z < d; ++z)
        set_pixel(x, y, z, r, g, b);
    }
    
    void fill(int value){
        std::fill(data, data + w * h * d * 3, value);
    }
    
    

    void set_pixel(int px, int py, int pz, int r, int g, int b){
        if ((px >= 0) && (px < w) && (py >= 0) && (py < h) && (pz >=0) && (pz < d))
        data[(pz * w * h + py * h + px) * 3] = r;
        data[(pz * w * h + py * h + px) * 3 + 1] = g;
        data[(pz * w * h + py * h + px) * 3 + 2] = b;
    }

    void set_pixel(int px, int py, int pz){
        if ((px >= 0) && (px < w) && (py >= 0) && (py < h) && (pz >=0) && (pz < d))
        set_pixel(px, py, pz, r, g, b);
    }

    void set_pixel(int px, int py, int pz, glm::vec3 v){
        set_pixel(px, py, pz, v.r * 255., v.g * 255., v.b * 255.);
    }

    void set_pixel(glm::ivec3 p, int r, int g, int b){
        set_pixel(p.x, p.y, p.z, r, g, b);
    }

    void set_pixel(glm::ivec3 p, glm::vec3 v){
        set_pixel(p.x, p.y, p.z, v);
    }

    void set_pixel(glm::ivec3 p){
        set_pixel(p, r, g, b);
    }

    void set_color(int r, int g, int b){
        this->r = r;
        this->g = g;
        this->b = b;
    }

    void set_color(glm::vec3 v){
        r = v.r * 255.;
        g = v.g * 255.;
        b = v.b * 255.;
    }
    void fill_circle(int px, int py, int pz, float radius){
        for (int x = px - (int)radius; x <= px + (int)radius; x++)
        for (int y = py - (int)radius; y <= py + (int)radius; y++)
        for (int z = pz - (int)radius; z <= pz + (int)radius; z++){
            if (glm::length(glm::vec3((px - x), (py - y), (pz - z))) <= radius)
            set_pixel(x, y, z);
        }
    }

    GLubyte* get_data(){
        return data;
    }
    ~TextureDrawer(){
        delete[] data;
    }
};




