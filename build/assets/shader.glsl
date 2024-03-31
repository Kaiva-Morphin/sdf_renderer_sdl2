#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

uniform float time;
uniform vec2 center;
layout(rgba8, binding = 0) writeonly uniform image2D destTex;

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }




struct ColorType{
    int type; // 0 - solid color; 1 - grad; 2 - noise grad; 3 - normal grad;
    int size; // size of values
    float colors[16][4];
    float values[16];
};

struct Primitive{
    int primitive_type; // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
    // universal
    float position[3];
    float translation_offset[3];
    float transform[3][3];
    float rounding;
    // specific points
    float a[3];
    float b[3];
    float c[3];
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





struct SDFResponse{
  vec3 color;
  float dist;
};


float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
  vec3 pa = p - a, ba = b - a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length( pa - ba*h ) - r;
}

float sdCappedCylinder( vec3 p, vec3 a, vec3 b, float r )
{
  vec3  ba = b - a;
  vec3  pa = p - a;
  float baba = dot(ba,ba);
  float paba = dot(pa,ba);
  float x = length(pa*baba-ba*paba) - r*baba;
  float y = abs(paba-baba*0.5)-baba*0.5;
  float x2 = x*x;
  float y2 = y*y*baba;
  float d = (max(x,y)<0.0)?-min(x2,y2):(((x>0.0)?x2:0.0)+((y>0.0)?y2:0.0));
  return sign(d)*sqrt(abs(d))/baba;
}

float dot2( in vec3 v ) { return dot(v,v); }

float udTriangle( vec3 p, vec3 a, vec3 b, vec3 c )
{
  vec3 ba = b - a; vec3 pa = p - a;
  vec3 cb = c - b; vec3 pb = p - b;
  vec3 ac = a - c; vec3 pc = p - c;
  vec3 nor = cross( ba, ac );

  return sqrt(
    (sign(dot(cross(ba,nor),pa)) +
     sign(dot(cross(cb,nor),pb)) +
     sign(dot(cross(ac,nor),pc))<2.0)
     ?
     min( min(
     dot2(ba*clamp(dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
     dot2(cb*clamp(dot(cb,pb)/dot2(cb),0.0,1.0)-pb) ),
     dot2(ac*clamp(dot(ac,pc)/dot2(ac),0.0,1.0)-pc) )
     :
     dot(nor,pa)*dot(nor,pa)/dot2(nor) );
}

layout (std430, binding = 1) buffer CellArrBuffer {
  PrimitiveScene scene;
};
mat3x3 mat3_from_array(float arr[3][3]){
  return mat3x3(arr[0][0], arr[0][1], arr[0][2], arr[1][0], arr[1][1], arr[1][2], arr[2][0], arr[2][1], arr[2][2]);
};

vec3 vec3_from_array(float arr[3]){
  return vec3(arr[0], arr[1], arr[2]);
};

float sdf_smoothunion(float d1, float d2, float k)
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

SDFResponse SampleScene(vec3 point){
  float mindist = 100000.;
  vec3 color = vec3(1., 0., 0.);
  for (int i=0;i<scene.size;i++) {
    Primitive primitive = scene.primitives[i];
    switch (primitive.primitive_type){
      case 0: //sphere
        mindist = min(sdf_smoothunion(sdSphere(vec3_from_array(primitive.position) - point, primitive.rounding), mindist, 25), mindist);
        color = vec3(1.);
        break;
      case 1: //capsule
        break;
      case 2: //box
        mindist = min(sdf_smoothunion(sdBox(vec3_from_array(primitive.position) - point, vec3_from_array(primitive.a)) - primitive.rounding, mindist, 25), mindist);
        color = vec3(1.);
        break;
      case 3: //cyl
        break;
      case 4: //triangle
        break;
      default:
        break;
    }
  }
  return SDFResponse(color, mindist);
}

const float min_dist = 1.;
const float max_dist = 1000.;


vec3 calcNormal( in vec3 p ) 
{
    const float eps = 0.001; 
    const vec2 h = vec2(eps,0);
    return normalize( vec3(SampleScene(p+h.xyy).dist - SampleScene(p-h.xyy).dist,
                           SampleScene(p+h.yxy).dist - SampleScene(p-h.yxy).dist,
                           SampleScene(p+h.yyx).dist - SampleScene(p-h.yyx).dist ) );
}



void main() {
  vec3 sun = vec3(1., -1., 1.);
  scene.primitives[0].rounding = 10.;
  scene.primitives[1].rounding = 10.;
  scene.primitives[0].position[0] = cos(time) * 100.;
  scene.primitives[0].position[1] = sin(time) * 100.;
  scene.size = 3;
  scene.primitives[2].position[1] = sin(time) * 100.;
  scene.primitives[2].a[0] = 50.;
  scene.primitives[2].a[1] = 50.;
  scene.primitives[2].a[2] = 50.;

  scene.primitives[2].transform[0][0] = 1.;
  scene.primitives[2].transform[1][0] = 0.;
  scene.primitives[2].transform[2][0] = 0.;
  scene.primitives[2].transform[0][1] = 0.;
  scene.primitives[2].transform[1][1] = 1.;
  scene.primitives[2].transform[2][1] = 0.;
  scene.primitives[2].transform[0][2] = 0.;
  scene.primitives[2].transform[1][2] = 0.;
  scene.primitives[2].transform[2][2] = 1.;

  scene.primitives[2].rounding = 5.;
  scene.primitives[2].primitive_type = 2;
  ivec2 storePos = ivec2(gl_GlobalInvocationID.xy); // pixel pos
  vec4 pixel = vec4(0., 0., 1., 255.);
  vec3 point = vec3(storePos.x - center.x, storePos.y - center.y, 1000.);
  vec3 direction = vec3(0., 0., -1.);
  int steps = 512;
  for (int i=0;i<steps;i++) {
    SDFResponse r = SampleScene(point);
    if (r.dist <= min_dist) {
      vec3 n = calcNormal(point);
      float cdn = clamp(dot(n, sun), 0.1, 1.);
      pixel.rgb = r.color.rgb * cdn;
      break;
    }

    if (r.dist > max_dist){
      pixel.rgb = vec3(0., 0., 0.);
    }
    point += direction * r.dist;
  }
  //pixel.r = 
  imageStore(destTex, storePos, pixel);
}