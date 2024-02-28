#include "cmath"
#include <vector>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

#define SDF_INF 10000.

/*struct Sphere{
    vec3 position;
    double radius;
};*/

/*enum ObjectType{
    Sphere,
    Box,
    RoundBox,
    BoxFrame,
    Torus,
    Plane,
    Capsule,
    Cylinder
};*/

class Object{
    public:
    vec3 position;
    mat3x3 rotation;
    virtual void draw(){};
    virtual double DistanceTo(vec3 positionm) {return 0.;};
};

class SphereObject : public Object {
    float radius;
    public:
    SphereObject(vec3 position, double radiu, mat3x3 rotation = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->position = position;
        this->radius = radius;
        this->rotation = rotation;
    };
    double DistanceTo(
        vec3 point
    ){
        return length(this->position - point) - this->radius;
    }
    void draw(){
        DrawCircle(position.z, position.x, radius);
    };
};

double sdf_blend(double d1, double d2, double a)
{
    return a * d1 + (1 - a) * d2;
}

double simple_min(double left, double right){
    if (left > right) {return right;};
    return left;
}

double simple_max(double left, double right){
    if (left > right) {return left;};
    return right;
}

vec3 maxvec3double(
    vec3 right,
    double left
){
    return vec3{simple_max(right.x, left), simple_max(right.y, left), simple_max(right.z, left)};
}

class BoxObject : public Object{
    vec3 size;
    public:
    BoxObject(vec3 position, vec3 size, mat3x3 rotation = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->rotation = rotation;
        this->position = position;
        this->size = size;
    };
    double DistanceTo(
        vec3 point
    ){

        vec3 vec = point - this->position;
        vec = vec * rotation;
        vec += this->position;

        vec3 q = abs(vec - this->position) - this->size;
        return length(
            maxvec3double(q,0.0)
            ) + simple_min(
                simple_max(
                    q.x,
                    simple_max(
                        q.y,
                        q.z
                    )
                ),
                0.0
            );
    }
    void draw(){

    };
};


struct Scene{
    vector<Object*> objects;
};

double SampleSceneSDF(
    vec3 point,
    Scene * scene
){
    double mindist = SDF_INF;
    for (size_t object_id = 0; object_id < scene->objects.size(); object_id++) {
        double dist = scene->objects[object_id]->DistanceTo(point);
        mindist = simple_min(dist, mindist);
    }
    return mindist;
}


double DrawSceneTD(
    Scene * scene
){
    double mindist = SDF_INF;
    for (size_t object_id = 0; object_id < scene->objects.size(); object_id++) {
        scene->objects[object_id]->draw();
    }
    return mindist;
}

std::tuple<vec3, double> RaySceneSDF(
    vec3 start,
    vec3 direction,
    Scene * scene,
    bool debug2d=false
){
    double min_dist = 0.1;
    double max_dist = 100.;
    direction = normalize(direction);
    vec3 position = start;
    int steps = 64;
    for (int step = 0; step < steps; ++step){
        double dist = SampleSceneSDF(position, scene); 
        if (dist <= min_dist) {
            if (debug2d){
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawPoint(renderer, position.z, position.x);
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawPoint(renderer, start.z, start.x);
            }
            return std::make_tuple(position, dist);
        };
        if (dist >= max_dist) {
            break;
        };
        vec3 step_vec = direction;
        if (debug2d){
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 90);
            DrawCircle(position.z, position.x, dist);
        }
        step_vec *= dist;
        if (debug2d){
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 90);
            SDL_RenderDrawLine(renderer, 
                (position).z, (position).x,
                (position + step_vec).z, (position + step_vec).x
            );
        }
        position += step_vec;
    }
    if (debug2d){
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawPoint(renderer, start.z, start.x);
    }
    return std::make_tuple(vec3{0, 0, 0}, SDF_INF);
}


vec3 CalculateNormal(vec3 position, Scene * scene){
    const double EPS = 0.001;
    vec3 v1 = vec3{
        SampleSceneSDF(position + vec3{EPS, 0, 0}, scene),
        SampleSceneSDF(position + vec3{0, EPS, 0}, scene),
        SampleSceneSDF(position + vec3{0, 0, EPS}, scene)
    };
    vec3 v2 = vec3{
        SampleSceneSDF(position - vec3{EPS, 0, 0}, scene),
        SampleSceneSDF(position - vec3{0, EPS, 0}, scene),
        SampleSceneSDF(position - vec3{0, 0, EPS}, scene)
    };
    return normalize(v1 - v2);
}


