#include <glm/glm.hpp>
using namespace glm;
#ifndef MY_FUNCTIONS
#define MY_FUNCTIONS


double remap(double value, double fromLow, double fromHigh, double toLow, double toHigh) {
    double normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

vec2 remap_vec2(vec2 value, vec2 fromLow, vec2 fromHigh, vec2 toLow, vec2 toHigh) {
    vec2 normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

mat4 roll_pitch_yaw_mat(float roll, float pitch, float yaw){
    return {
        cos(roll) * cos(pitch),                           -cos(pitch)*cos(roll),                                  sin(pitch),          0,
        cos(yaw)*sin(roll)+cos(roll)*sin(pitch)*sin(yaw),  cos(roll) * cos(yaw) - sin(roll)*sin(pitch)*sin(yaw), -cos(pitch)*sin(yaw), 0,
        -cos(roll)*cos(yaw)*sin(pitch)+sin(roll)*sin(yaw), cos(yaw)*sin(roll)*sin(pitch)+cos(roll)*sin(yaw),      cos(pitch)*cos(yaw), 0,
        0, 0, 0, 1
    };
}

class Game;
Game* game = nullptr;


#endif