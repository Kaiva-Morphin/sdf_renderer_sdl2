#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

layout(rgba8, binding = 0) writeonly uniform image2D destTex;

struct Primitive {
    mat4x4 transform;
    vec4 texture_position;
    vec4 position;
    float primitive_type;
    float rounding;
    //vec3 translation_offset;
    //vec3 a;
    //vec3 b;
    //vec3 c;
};

layout(std140, binding = 1) buffer PrimitivesBuffer {
    Primitive primitives[16];
};


void main() {
  ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
  vec4 pixel = vec4(0.0863, 0.1765, 0.2549, 1.0);
  pixel.rgb = vec3(primitives[14].rounding == 555 ? 1. : 0.);
  imageStore(destTex, storePos, pixel);
}