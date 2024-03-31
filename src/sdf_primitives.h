#include "cmath"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
using namespace glm;
using namespace std;


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

struct ColorType{
    int type; // 0 - solid color; 1 - grad; 2 - noise grad; 3 - normal grad;
    int size; // size of values
    vec4 colors[16];
    float values[16];
};
struct Primitive{
    int primitive_type; // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
    // universal
    vec3 position;
    vec3 translation_offset;
    mat3x3 transform;
    float rounding;
    // specific points
    vec3 a;
    vec3 b;
    vec3 c;
};

struct ColorSpace{
    bool global; // is global or applyed with parent transform
    int size;
    Primitive primitives[32];
    ColorType colors[32];
};

struct PrimitiveOperation{
    int operation_type; // 0 - soft merge, 1 - subtract, 2 - intersection, 3 - xor
    int size;
    int objects[16];
    float value;
};

struct PrimitiveScene{
    int size;
    Primitive primitives[4];
    PrimitiveOperation operations[16];
};



class Object{
    public:
    vec3 position;
    float rounding;
    vec4 color = vec4{1, 1, 1, 1};
    vec3 translation_offset = vec3{0, 0, 0};
    mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1};
    Primitive as_primitive();
};

struct ObjectColorSpace{
    bool global; // is global or applyed with parent transform
    unsigned int size;
    Object objects[32];
    ColorType colors[32];
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
            position,
            translation_offset,
            transform,
            rounding,
            vec3(0.),
            vec3(0.),
            vec3(0.),
        };
    }
    
};

class BoxObject : public Object {
    vec3 size;
    public:
    BoxObject(vec3 position, vec3 size, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->translation_offset = vec3(0);
        this->position = position;
        this->size = size;
    };
    Primitive as_primitive(){
        return Primitive{
            2, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            position,
            translation_offset,
            transform,
            rounding,
            size,
            vec3(0.),
            vec3(0.),
        };
    }
};

class LineObject : public Object{
    vec3 p1;
    vec3 p2;
    public:
    LineObject(vec3 position, vec3 p1, vec3 p2, float radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->translation_offset = vec3(0);
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            1, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            position,
            translation_offset,
            transform,
            rounding,
            p1,
            p2,
            vec3(0.),
        };
    }
};

class Cylinder : public Object{
    vec3 p1;
    vec3 p2;
    public:
    Cylinder(vec3 position, vec3 p1, vec3 p2, float radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->translation_offset = vec3(0);
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            3, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            position,
            translation_offset,
            transform,
            rounding,
            p1,
            p2,
            vec3(0.),
        };
    }
};

class Triangle : public Object{
    vec3 p1;
    vec3 p2;
    vec3 p3;
    public:
    Triangle(vec3 position, vec3 p1, vec3 p2, vec3 p3, float radius, mat3x3 transform = mat3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}){
        this->transform = transform;
        this->translation_offset = vec3(0);
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            4, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            position,
            translation_offset,
            transform,
            rounding,
            p1,
            p2,
            p3,
        };
    }
};


