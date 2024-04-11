#include "cmath"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
using namespace glm;
using namespace std;

#define SDF_INF 10000.
#define eye3 mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}
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
#include "fast_noise.h"
FastNoiseLite noise;

class Object{
    public:
    vec3 position;
    vec4 color = vec4{1, 1, 1, 1};
    vec3 translation_offset = vec3{0, 0, 0};
    mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1};

    vec4 GetColor(vec3 point){
        point = applyRelativeTransforms(point);
        double val = noise.GetNoise(point.x, point.y, point.z);
        val = (val + 1) * 0.5;
        if (val * 255. < 170.) {
            return vec4{150. / 255., 150. / 255., 0., 1.};
        } else {
            return vec4{40. / 255., 40. / 255., 0., 1.};
        }
         
    }

    vec3 applyRelativeTransforms(vec3 point){
        vec3 vec = point - this->position - this->translation_offset;
        vec = vec * transform;
        vec += this->position;
        return vec;
    }
    virtual void draw(){};
    virtual double DistanceTo(vec3 position) {return 0.;};
};

class SphereObject : public Object {
    float radius;
    public:
    SphereObject(vec3 position, double radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->position = position;
        this->translation_offset = vec3(0);
        this->radius = radius;
        this->transform = transform;
    };
    double DistanceTo(
        vec3 point
    ){
        return length(this->position - applyRelativeTransforms(point)) - this->radius;
    }
    void draw(){
        //DrawCircle(this->position.z, this->position.x, this->radius);
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
    BoxObject(vec3 position, vec3 size, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->translation_offset = vec3(0);
        this->position = position;
        this->size = size;
    };
    double DistanceTo(
        vec3 point
    ){
        vec3 q = abs(applyRelativeTransforms(point) - this->position) - this->size;
        return length(maxvec3double(q,0.0)) + simple_min(simple_max(q.x,simple_max(q.y,q.z)),0.0);
    }
    void draw(){

    };
};

class LineObject : public Object{
    vec3 p1;
    vec3 p2;
    double radius;
    public:
    LineObject(vec3 position, vec3 p1, vec3 p2, double radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->translation_offset = vec3(0);
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->radius = radius;
    };
    double DistanceTo(
        vec3 point
    ){
        point = applyRelativeTransforms(point);
        vec3 pa = point - position - p1;
        vec3 ba = p2 - p1;
        double h = glm::clamp( (double)(dot(pa,ba)/dot(ba,ba)), 0.0, 1.0 );
        ba *= h;
        return length( pa - ba ) - radius;
    }
    void draw(){};
};



struct Scene{
    vector<Object*> objects;
};

float sdf_blend(float d1, float d2, float a)
{
    return a * d1 + (1 - a) * d2;
}

float sdf_smoothunion(float d1, float d2, float k)
{
    float h = simple_max(k-abs(d1-d2),0.0);
    return simple_min(d1, d2) - h*h*0.25/k;
}

std::tuple<double, vec4> SampleSceneSDF(
    vec3 point,
    Scene * scene
){
    double mindist = SDF_INF;
    vec4 res_color;
    vector<double> distances;
    for (size_t object_id = 0; object_id < scene->objects.size(); object_id++) {
        double dist = scene->objects[object_id]->DistanceTo(point);
        vec4 color = scene->objects[object_id]->color;
        distances.push_back(dist);
        if (mindist == SDF_INF){
            mindist = distances[object_id];
            res_color = color;
        } else {
            /*
            res_color.r = mindist < distances[object_id] ? res_color.r : color.r;
            res_color.g = mindist < distances[object_id] ? res_color.g : color.g;
            res_color.b = mindist < distances[object_id] ? res_color.b : color.b;
            */
           
            
            float interpolation = clamp(0.5 + 0.5 * (mindist - distances[object_id]) / 15., 0.0, 1.0);
            res_color =  mix(res_color, color, interpolation);
            mindist = sdf_smoothunion(mindist, distances[object_id], 25);
            
        }
        //mindist = simple_min(dist, mindist);
    }
    //double merged_dist = sdf_smoothunion(distances[0], distances[1], 15.);
    //merged_dist = sdf_smoothunion(merged_dist, distances[2], 15);
    //merged_dist = sdf_smoothunion(merged_dist, distances[3], 15);
    //cout << distances[3] << endl;
    //mindist = sdf_smoothunion(distances[0], distances[1], 20.);
    //mindist = simple_min(distances[2], mindist);
    return std::make_tuple(mindist, res_color);
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

std::tuple<vec3, double, vec4> RaySceneSDF(
    vec3 start,
    vec3 direction,
    Scene * scene,
    bool debug2d=false
){
    double min_dist = 1;
    double max_dist = 100.;
    direction = normalize(direction);
    vec3 position = start;
    int steps = 64;
    for (int step = 0; step < steps; ++step){
        double dist;
        vec4 c; 
        tie(dist, c) = SampleSceneSDF(position, scene); 
        if (dist <= min_dist) {
            if (debug2d){
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawPoint(renderer, position.z, position.x);
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawPoint(renderer, start.z, start.x);
            }
            return std::make_tuple(position, dist, c);
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
    return std::make_tuple(vec3{0, 0, 0}, SDF_INF, vec4{});
}


vec3 CalculateNormal(vec3 position, Scene * scene){
    const double EPS = 0.001;
    vec3 v1 = vec3{
        std::get<0>(SampleSceneSDF(position + vec3{EPS, 0, 0}, scene)),
        std::get<0>(SampleSceneSDF(position + vec3{0, EPS, 0}, scene)),
        std::get<0>(SampleSceneSDF(position + vec3{0, 0, EPS}, scene))
    };
    vec3 v2 = vec3{
        std::get<0>(SampleSceneSDF(position - vec3{EPS, 0, 0}, scene)),
        std::get<0>(SampleSceneSDF(position - vec3{0, EPS, 0}, scene)),
        std::get<0>(SampleSceneSDF(position - vec3{0, 0, EPS}, scene))
    };
    return normalize(v1 - v2);
}


std::tuple<int, int, int, int> compute_pixel(int x, int y, vec3 pos, vec3 direction, Scene * scene, vec3 sun_vector){
    vec3 hit_point;
    double result;
    vec4 color;
    tie(hit_point, result, color) = RaySceneSDF(pos, direction, scene);
    if (result < SDF_INF){
        vec3 normal = CalculateNormal(hit_point, scene);
        double dot_product = 0.1 + 0.9 * (dot(normalize(sun_vector), normal) * 0.5 + 0.5);
        double amp = glm::clamp(dot_product, 0., 1.);
        return std::make_tuple(round(color.r * amp * 255), round(color.g * amp * 255), round(color.b * amp * 255), round(color.a * 255));
    } else {
        return std::make_tuple(0, 0, 0, 0);
    }
}

void compute_row(int x, int ysize, Scene *scene, vec3 sun_vector, uint32_t *result){
    for (int y = 0; y < ysize; ++y){
        vec3 pos = vec3{x + 30, y - 30, 1};
        vec3 direction = vec3{0, 0, 1}; // orthoganal camera
        int r,g,b,a;

        tie(r, g, b, a) = compute_pixel(x, y, pos, direction, scene, sun_vector);
        //[x + y * TARGET_WIDTH]
        //if (a == 0){continue;}
        result[y] = (static_cast<uint32_t>(r) << 24) |
                                        (static_cast<uint32_t>(g) << 16) |
                                        (static_cast<uint32_t>(b) << 8) |
                                        static_cast<uint32_t>(a);
    }
}