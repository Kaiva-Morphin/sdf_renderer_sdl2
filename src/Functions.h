#include <glm/glm.hpp>
using namespace glm;
#ifndef MY_FUNCTIONS
#define MY_FUNCTIONS

#define EYE4 {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}

mat4x4 EulerXYZ(float anglex, float angley, float anglez){
    anglex = anglex / 180. * 3.1415;
    angley = angley / 180. * 3.1415;
    anglez = anglez / 180. * 3.1415;
    mat4x4 rotmatx = mat4x4(
           vec4(1., 0., 0., 0.),
           vec4(0., cos(anglex), -sin(anglex), 0.),
           vec4(0., sin(anglex), cos(anglex), 0.),
           vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmaty = mat4x4(
            vec4(cos(angley), 0., sin(angley), 0.),
            vec4(0., 1., 0., 0.),
            vec4(-sin(angley), 0., cos(angley), 0.),
            vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmatz = mat4x4(
            vec4(cos(anglez), -sin(anglez), 0., 0.),
            vec4(sin(anglez), cos(anglez), 0., 0.),
            vec4(0., 0., 1., 0.),
            vec4(0., 0., 0., 1.)
    );
    return rotmatx * rotmaty * rotmatz;
}

mat4x4 EulerZYX(float anglex, float angley, float anglez){
    anglex = anglex / 180. * 3.1415;
    angley = angley / 180. * 3.1415;
    anglez = anglez / 180. * 3.1415;
    mat4x4 rotmatx = mat4x4(
           vec4(1., 0., 0., 0.),
           vec4(0., cos(anglex), -sin(anglex), 0.),
           vec4(0., sin(anglex), cos(anglex), 0.),
           vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmaty = mat4x4(
            vec4(cos(angley), 0., sin(angley), 0.),
            vec4(0., 1., 0., 0.),
            vec4(-sin(angley), 0., cos(angley), 0.),
            vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmatz = mat4x4(
            vec4(cos(anglez), -sin(anglez), 0., 0.),
            vec4(sin(anglez), cos(anglez), 0., 0.),
            vec4(0., 0., 1., 0.),
            vec4(0., 0., 0., 1.)
    );
    return rotmatz * rotmaty * rotmatx;
}

double remap(double value, double fromLow, double fromHigh, double toLow, double toHigh) {
    double normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

vec2 remap_vec2(vec2 value, vec2 fromLow, vec2 fromHigh, vec2 toLow, vec2 toHigh) {
    vec2 normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

class Game;
Game* game = nullptr;

#endif