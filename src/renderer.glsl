#version 430

layout (local_size_x = 16, local_size_y = 16) in;

uniform float time;
layout(rgba8, binding = 0) writeonly uniform image2D destTex;

void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    vec4 pixel = vec4(sin(float(storePos.x) * 0.1 + time) * 0.5 + 0.5, cos(float(storePos.y) * 0.1 + time) * 0.5 + 0.5, 0.0, 1.0);
    imageStore(destTex, storePos, pixel);
}