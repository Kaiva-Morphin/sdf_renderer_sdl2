#include "cmath"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
using namespace glm;
using namespace std;


#ifndef SDF_INITED
#define SDF_INITED


/*

Scene -> "Bones" -> Primitives 
Primitive hierarhy

Scene is up to 64 primitives.

Object.as_primitive();
primitives: everything with rounding
    sphere (dot)
    capsule (line)
    box
    triangle
    cyl

every object contains colorspace

Colorspace is up to 32 colored objects
every object can be:
    color;
    gradient;
    noise grad;
    normal gradient;

bloom?

*/

struct Primitive{
    int primitive_type; // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
    // universal
    float texture_position[3];
    float position[3]; // where is center
    float translation_offset[3]; // where is rotation point relative to center
    float transform[3][3]; // rotation and scale
    float rounding;
    // specific points
    float a[3];
    float b[3];
    float c[3];
};

struct PrimitiveOperation{
    int operation_type; // 0 - soft add, 1 - soft subtract, 2 - intersection, 3 - xor
    int a;
    int b;
    float value;
};

struct PrimitiveScene{
    int size;
    Primitive primitives[32];
    PrimitiveOperation operations[16];
};

class Object{
    public:
    vec3 position = vec3(0.);
    float rounding = 0.;
    vec3 translation_offset = vec3{0, 0, 0};
    mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1};
    virtual Primitive as_primitive(){return Primitive{};};
};

class SphereObject : public Object {
    public:
    SphereObject(vec3 position, float radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->position = position;
        this->translation_offset = vec3(0);
        this->rounding = radius;
        this->transform = transform;
    };
    Primitive as_primitive(){
        return Primitive{
            0, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            {0., 0., 0.,},
            {position.x, position.y, position.z},
            {translation_offset.x, translation_offset.y, translation_offset.z},
            {{transform[0][0], transform[0][1], transform[0][2]}, {transform[1][0], transform[1][1], transform[1][2]}, {transform[2][0], transform[2][1], transform[2][2]}},
            rounding,
            {0., 0., 0.,},
            {0., 0., 0.,},
            {0., 0., 0.,}
        };
    }
    
};

class BoxObject : public Object {
    vec3 size;
    public:
    BoxObject(vec3 position, vec3 size, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->position = position;
        this->size = size;
    };
    Primitive as_primitive(){
        return Primitive{
            2, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            {0., 0., 0.,},
            {position.x, position.y, position.z},
            {translation_offset.x, translation_offset.y, translation_offset.z},
            {{transform[0][0], transform[0][1], transform[0][2]}, {transform[1][0], transform[1][1], transform[1][2]}, {transform[2][0], transform[2][1], transform[2][2]}},
            rounding,
            {size.x, size.y, size.z},
            {0., 0., 0.,},
            {0., 0., 0.,}
        };
    }
};

class LineObject : public Object{
    vec3 p1;
    vec3 p2;
    public:
    LineObject(vec3 position, vec3 p1, vec3 p2, float radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            1, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            {0., 0., 0.,},
            {position.x, position.y, position.z},
            {translation_offset.x, translation_offset.y, translation_offset.z},
            {{transform[0][0], transform[0][1], transform[0][2]}, {transform[1][0], transform[1][1], transform[1][2]}, {transform[2][0], transform[2][1], transform[2][2]}},
            rounding,
            {p1.x, p1.y, p1.z},
            {p2.x, p2.y, p2.z},
            {0., 0., 0.,}
        };
    }
};

class CylinderObject : public Object{
    vec3 p1;
    vec3 p2;
    public:
    CylinderObject(vec3 position, vec3 p1, vec3 p2, float radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            3, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            {0., 0., 0.,},
            {position.x, position.y, position.z},
            {translation_offset.x, translation_offset.y, translation_offset.z},
            {{transform[0][0], transform[0][1], transform[0][2]}, {transform[1][0], transform[1][1], transform[1][2]}, {transform[2][0], transform[2][1], transform[2][2]}},
            rounding,
            {p1.x, p1.y, p1.z},
            {p2.x, p2.y, p2.z},
            {0., 0., 0.,}
        };
    }
};

class TriangleObject : public Object{
    vec3 p1;
    vec3 p2;
    vec3 p3;
    public:
    TriangleObject(vec3 position, vec3 p1, vec3 p2, vec3 p3, float radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            4, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            {0., 0., 0.,},
            {position.x, position.y, position.z},
            {translation_offset.x, translation_offset.y, translation_offset.z},
            {{transform[0][0], transform[0][1], transform[0][2]}, {transform[1][0], transform[1][1], transform[1][2]}, {transform[2][0], transform[2][1], transform[2][2]}},
            rounding,
            {p1.x, p1.y, p1.z},
            {p2.x, p2.y, p2.z},
            {p3.x, p3.y, p3.z},
        };
    }
};


class ObjectScene {
    public:
    vector<Object*> objects;
    void update_primitive_scene(PrimitiveScene* scene){
        scene->size = objects.size();
        for (int i = 0; i < scene->size; i++){
            scene->primitives[i] = objects[i]->as_primitive();
        }
    };
};

#endif