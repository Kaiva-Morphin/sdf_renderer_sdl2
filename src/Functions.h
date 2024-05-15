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

class Game;
Game* game = nullptr;


#endif