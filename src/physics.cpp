#include <SDL2/SDL.h>
#include "graphics.h"
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <vector>
using namespace std;
using namespace glm;
#define PI 3.14


void DrawCircle(int32_t centreX, int32_t centreY, int32_t radius, bool is_header)
{
   const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      if (is_header){
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      } else {
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
      }
      

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}

void drawCapsule(int x, int y, int radius, int length) {
    SDL_RenderDrawLine(renderer, x - radius, y + length, x - radius, y - length);
    SDL_RenderDrawLine(renderer, x + radius, y + length, x + radius, y - length);
    DrawCircle(x, y + length, radius, false);
    DrawCircle(x, y - length, radius, true);
}

vec2 rotate_vec(vec2 vec, float angle){
    vec2 rotated;
    rotated.x = vec.x*cos(angle) - vec.y*sin(angle);
    rotated.y = vec.x*sin(angle) + vec.y*cos(angle);
    return rotated;
}

class AABB {
    public:
    AABB(const glm::vec2& min_, const glm::vec2& max_) : min(min_), max(max_) {}

    bool intersects(const AABB& other) const {
        return glm::all(glm::lessThanEqual(min, other.max)) && glm::all(glm::greaterThanEqual(max, other.min));
    }
    bool is_point_inside(vec2 point){
        return glm::all(glm::lessThanEqual(min, point)) && glm::all(glm::greaterThanEqual(max, point));
    }
    glm::vec2 min, max;
};


vec2 apply_rounding(vec2 position, vec2 destination, float rounding){
    if (rounding == 0.){return position;};
    if (destination == position){return position;};
    vec2 vector = destination - position;
    return position + normalize(vector) * rounding;
};

class PhysicsObject{
    public:
    float mass=1.;
    float rounding = 0.;
    bool is_fixed=true;
    vec2 position=vec2(0., 0.);
    vec2 velocity=vec2(0., 0.);
    virtual vec2 center(){return position;};
    virtual AABB get_binding_box(){return AABB(vec2(), vec2());};
    virtual void draw(){};
    virtual vec2 closest_point(vec2 second_center){return vec2(0, 0);};
};

class PhysicsScene{
    public:
    vec2 gravity_vector = vec2(0., 1000.);
    vector<PhysicsObject*> objects;
    PhysicsScene(){}
    void step(float deltaTime){
        for (auto object : objects){
            if (!object->is_fixed){
                object->velocity += gravity_vector * deltaTime;
                object->position += object->velocity * deltaTime;
            }
            for (auto second_object : objects){
                if (object == second_object){continue;}
                if (object->is_fixed && second_object->is_fixed){continue;}
                if (object->get_binding_box().intersects(second_object->get_binding_box())) {
                    vec2 center1 = object->center();
                    vec2 center2 = second_object->center();
                    vec2 closest_point1 = object->closest_point(center2);
                    vec2 closest_point2 = second_object->closest_point(closest_point1);
                    vec2 result1 = apply_rounding(closest_point1, closest_point2, object->rounding);
                    vec2 result2 = apply_rounding(closest_point2, closest_point1, second_object->rounding);
                    vec2 vec = result1 - result2;
                    if (dot(vec, center1 - center2) > 0) {
                        continue;
                    }
                    if (object->is_fixed){
                        second_object->position += vec;
                        second_object->velocity = second_object->velocity - 2 * dot(second_object->velocity, vec) * vec;
                        second_object->velocity *= 0.9995;
                    } else if (second_object->is_fixed){
                        object->position -= vec;
                        object->velocity = object->velocity - 2 * dot(object->velocity, -vec) * -vec;
                        object->velocity *= 0.9995;
                    } else {
                        object->position -= vec * 0.5f;
                        object->velocity = object->velocity - 2 * dot(object->velocity, -vec) * -vec;
                        object->velocity *= 0.9995;
                        second_object->position += vec * 0.5f;
                        second_object->velocity = second_object->velocity - 2 * dot(second_object->velocity, vec) * vec;
                        second_object->velocity *= 0.9995;
                    }
                }
            }
        }
    }
    void draw(){
        for (auto object : objects){
            if (object->is_fixed){SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);}
            else {SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);}
            object->draw();
            SDL_RenderDrawLine(renderer, object->position.x, object->position.y, object->position.x + object->velocity.x * 0.1, object->position.y + object->velocity.y * 0.1);
        }
    }
};
PhysicsScene scene = PhysicsScene();

vec2 vec2max(vec2 vec, float mx){
    return vec2(std::max(vec.x, mx), std::max(vec.y, mx));
}
vec2 vec2min(vec2 vec, float mx){
    return vec2(std::min(vec.x, mx), std::min(vec.y, mx));
}
float simple_max(float left, float right){
    if (right > left){return right;}
    return left;
}
class Box : public PhysicsObject{
    public:
    vec2 size;
    Box(vec2 size=vec2(10, 10), bool is_fixed = false){
        this->size = size;
        this->is_fixed = is_fixed;
    }
    AABB get_binding_box(){
        vec2 half = size;
        half /= 2;
        return AABB(position - half, position + half);
    }
    void draw(){
        SDL_Rect rect = SDL_Rect{(int)(this->position.x - this->size.x * 0.5), (int)(this->position.y - this->size.y * 0.5), (int)(this->size.x), (int)(this->size.y)};
        SDL_RenderDrawRect(renderer, &rect);
    }
    vec2 closest_point(vec2 second_center){
        AABB aabb = get_binding_box();
        float x = second_center.x;
        float y = second_center.y;
        if (aabb.is_point_inside(second_center)){
            float dx1 = abs(x - aabb.min.x);
            float dx2 = abs(x - aabb.max.x);
            float dy1 = abs(y - aabb.min.y);
            float dy2 = abs(y - aabb.max.y);
            if (std::min(dx1, dx2) < std::min(dy1, dy2)){
                if (dx1 > dx2){
                    x = aabb.max.x;
                } else {
                    x = aabb.min.x;
                }
                return vec2(x, y);
            } else {
                if (dy1 > dy2){
                    y = aabb.max.y;
                } else {
                    y = aabb.min.y;
                }
                return vec2(x, y);
            }
            return vec2(x, y);
        }
        x = std::min(x, aabb.max.x);
        x = std::max(x, aabb.min.x);
        y = std::min(y, aabb.max.y);
        y = std::max(y, aabb.min.y);
        return vec2(x, y);
    }
};

class Triangle : public PhysicsObject{
    
};

class Capsule : public PhysicsObject{
    public:
    float height; // todo: rewrite to halfs.
    Capsule(float height=20., float radius=10., bool is_fixed = false){
        this->height = height;
        this->rounding = radius;
        this->is_fixed = is_fixed;
        scene.objects.push_back(this);
    }
    AABB get_binding_box(){
        vec2 offset = vec2(rounding, rounding + height * 0.5);
        return AABB(position - offset, position + offset);
    }
    void draw(){
        drawCapsule(this->position.x, this->position.y, this->rounding, this->height*0.5f);
    }
    vec2 closest_point(vec2 second_center){ // todo: exception for zeros
        AABB aabb = get_binding_box();
        float x = position.x;
        float y = second_center.y;
        y = std::min(y, position.y+height*0.5f);
        y = std::max(y, position.y-height*0.5f);
        return vec2(x, y);
    }
};


int xMouse, yMouse;
int main(int argc, char* argv[]) {
    init();

    Box floor = Box();
    floor.is_fixed = true;
    floor.size = vec2(200., 10.);
    floor.position = vec2(100., 80.1);
    scene.objects.push_back(&floor);

    Box floor2 = Box();
    floor2.is_fixed = true;
    floor2.size = vec2(360., 10.);
    floor2.position = vec2(180., 220.1);
    scene.objects.push_back(&floor2);

    Box wall = Box();
    wall.is_fixed = true;
    wall.size = vec2(10., 70.);
    wall.position = vec2(10., 100.);
    scene.objects.push_back(&wall);
    
    Box box = Box();
    box.is_fixed = false;
    box.size = vec2(20., 20.);
    box.position = vec2(70., 70.);
    scene.objects.push_back(&box);


    Capsule player = Capsule();
    player.position = vec2(130., 75.);
    player.is_fixed = false;
    scene.objects.push_back(&player);

    Capsule c = Capsule();
    c.position = vec2(40., 10.);
    c.is_fixed = false;
    scene.objects.push_back(&c);

    Capsule capsule = Capsule();
    capsule.is_fixed = true;
    capsule.position = vec2(240., 120.);
    capsule.rounding = 20.;
    capsule.height = 0.;
    scene.objects.push_back(&capsule);

    Uint32 currentTime = SDL_GetTicks();
    Uint32 lastTime = currentTime;
    float deltaTime = 0.0f;
    bool sim = false;
    bool key_a, key_d, key_w;
    key_a = false;
    key_d = false;
    key_w = false;
    bool done = false;
    while (!done)
    {
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        clear_screen(240, 230, 230);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        while (SDL_PollEvent(&event)){
            switch (event.type)
            {
                case SDL_QUIT:
                    done = true;
            }
            if(event.type == SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&xMouse,&yMouse);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                SDL_GetMouseState(&xMouse,&yMouse);
                cout << "new" << endl;
                Box *zxc = new Box();
                zxc->is_fixed = false;
                zxc->size = vec2(30., 30.);
                int w, he;
                SDL_GetWindowSizeInPixels(window, &w, &he);
                int xm = xMouse / ((float)w  / (float)TARGET_WIDTH);
                int ym = yMouse / ((float)he / (float)TARGET_HEIGHT);
                zxc->position = vec2(xm, ym);
                scene.objects.push_back(zxc);
            }
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
                    case SDLK_SPACE:
                        sim = !sim;
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
                }
            }
        }
        if (key_a) player.velocity.x = -90;
        if (key_d) player.velocity.x = 90;
        if (key_w) player.velocity.y = -300;
        
        
        



        int w, he;
        SDL_GetWindowSizeInPixels(window, &w, &he);
        int xm = xMouse / ((float)w  / (float)TARGET_WIDTH);
        int ym = yMouse / ((float)he / (float)TARGET_HEIGHT);
        vec2 pos = vec2(xm, ym);
        if (!sim){
            player.position = vec2(xm, ym); 
        }
        

        //scene.step(deltaTime);

        /*PhysicsObject* object = &wall;
        PhysicsObject* second_object = &player;
        vec2 center1 = object->center();
        vec2 center2 = second_object->center();
        vec2 closest_point1 = object->closest_point(center2);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, closest_point1.x, closest_point1.y, center1.x, center1.y);
        
        vec2 closest_point2 = second_object->closest_point(closest_point1);
        vec2 result1 = apply_rounding(closest_point1, closest_point2, object->rounding);
        vec2 result2 = apply_rounding(closest_point2, closest_point1, second_object->rounding);
        vec2 vec = result1 - result2;
        if (dot(vec, center1 - center2) > 0) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        }
        SDL_RenderDrawLine(renderer, result1.x, result1.y, result2.x, result2.y);*/
        if (sim){scene.step(deltaTime);}
        /*if (object->is_fixed){
            second_object->position += vec;
            second_object->velocity.y = 0.;
        } else if (second_object->is_fixed){
            object->position -= vec;
            object->velocity.y = 0.;
        } else {
            object->position += vec * 0.5f;
            object->velocity.y = 0.;
            second_object->position -= vec * 0.5f;
            second_object->velocity.y = 0.;
        }*/
        scene.draw();
        draw();
    }
    cleanup();
    return 0;
}