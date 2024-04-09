#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

uniform float time;
uniform vec2 center;
layout(rgba8, binding = 0) writeonly uniform image2D destTex;


struct Primitive{
    mat4x4 transform;
    mat4x4 texture_transform;
    vec4 position;
    vec4 a;
    vec4 b;
    vec4 c;
    int primitive_type; // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
    float rounding;
};

struct PrimitiveOperation{
    int operation_type; // 0 - soft merge, 1 - subtract, 2 - intersection, 3 - xor
    int a;
    int b;
    float value;
};

struct PrimitiveScene{
    int size;
    Primitive primitives[32];
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



layout (std140, binding = 1) buffer SceneBuffer {
  PrimitiveScene scene;
};




mat3x3 mat4x4_to_mat3x3(mat4x4 mat) {
    return mat3x3(mat[0].xyz, mat[1].xyz, mat[2].xyz);
}


float sdf_smoothunion(float d1, float d2, float k)
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

uniform sampler3D character_texture;

vec3 in_primitive_space(vec3 point, Primitive primitive){
  mat4x4 transform = -primitive.transform;
  return (transform * (vec4(point - primitive.position.xyz, 1.))).xyz;
}

SDFResponse SampleScene(vec3 point){
  float mindist = 100000.;
  float new_dist = 0.;
  vec3 color = vec3(1., 0., 0.);
  for (int i=0;i<scene.size;i++) {
    Primitive primitive = scene.primitives[i];
    //vec3 tex_color = vec3(sin(i / 3.14), cos(i / 3.14), sin(i));
    vec3 tex_color = texture(character_texture,  vec3(0.5) - (primitive.texture_transform * vec4(in_primitive_space(point, primitive), 1.)).xyz).rgb; // 0.1 scale ~ 5. render_units && 1 scale ~ 0.5 render_units
    switch (primitive.primitive_type){
      case 0: //sphere
        new_dist = sdSphere(in_primitive_space(point, primitive), primitive.rounding);
        break;
      case 1: //capsule
        new_dist = sdCapsule(
            in_primitive_space(point, primitive),
            -primitive.a.xyz,
            -primitive.b.xyz,
            primitive.rounding
          ), mindist;
        break;
      case 2: //box
        new_dist = sdBox(in_primitive_space(point, primitive), primitive.a.xyz) - primitive.rounding;
        break;
      case 3: //cyl
        new_dist = sdCappedCylinder(
            in_primitive_space(point, primitive),
            -primitive.a.xyz,
            -primitive.b.xyz,
            primitive.rounding
          );
        break;
      case 4: //triangle
        new_dist = 
          udTriangle(
            in_primitive_space(point, primitive),
            -primitive.a.xyz,
            -primitive.b.xyz,
            -primitive.c.xyz
        ) - primitive.rounding;
        break;
      default:
        break;
    }
    float interpolation = clamp(0.5 + 0.5 * (mindist - new_dist) / 4.1, 0.0, 1.0);
    mindist = sdf_smoothunion(mindist, new_dist, 2.7);
    color = mix(color, tex_color, interpolation);
  }
  return SDFResponse(color, mindist);
}

const float near_z = 10.;
const float far_z = -10.;
const float min_dist = 0.01;
const float max_dist = abs(near_z) + abs(far_z) + 1;
const int steps = 128;

vec3 calcNormal( in vec3 p ) 
{
    const float eps = 0.001; 
    const vec2 h = vec2(eps,0);
    return normalize( vec3(SampleScene(p+h.xyy).dist - SampleScene(p-h.xyy).dist,
                           SampleScene(p+h.yxy).dist - SampleScene(p-h.yxy).dist,
                           SampleScene(p+h.yyx).dist - SampleScene(p-h.yyx).dist ) );
}

float calcSoftshadow( in vec3 ro, in vec3 rd, float tmin, float tmax, const float k )
{
	float res = 1.0;
    float t = tmin;
    for( int i=0; i<50; i++ )
    {
		float h = SampleScene( ro + rd*t ).dist;
        res = min( res, k*h/t );
        t += clamp( h, 0.02, 0.20 );
        if( res<0.005 || t>tmax ) break;
    }
    return clamp( res, 0.0, 1.0 );
}

const float scale_factor = 0.1;

/*

//*     Y
//*     |
//*     * -- X
//*      \
//*        Z

*/

void main() {
  ivec2 storePos = ivec2(gl_GlobalInvocationID.xy); // pixel pos
  vec4 pixel = vec4(0.0863, 0.1765, 0.2549, 1.0);
  vec3 sun = vec3(1., 1., 1.); // sun direction
  


  if (true) for (int i = 0; i < scene.size; i++){
    scene.primitives[i].transform[0] = vec4(1., 0., 0., 0.);
    scene.primitives[i].transform[1] = vec4(0., 1., 0., 0.);
    scene.primitives[i].transform[2] = vec4(0., 0., 1., 0.);
    //scene.primitives[i].transform[0] = vec4(0., 0., 1., 0.);
  }
  /*scene.primitives[0].transform[3] = vec4(0., 0., 0., 1.);
  scene.primitives[0].position = vec4(5., 0., 0., 0.);
  scene.primitives[0].texture_transform = mat4x4(
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.
  );*/
  
  vec3 start = vec3((storePos.x - center.x) * scale_factor, (storePos.y - center.y) * -scale_factor, near_z);
  vec3 point = start;
  vec3 direction = vec3(0., 0., -1.);
  for (int i=0;i<steps;i++) {
    SDFResponse r = SampleScene(point);
    if (r.dist > max_dist || point.z <= far_z){
      //pixel.rgb = vec3(0.);
      break;
    }
    if (r.dist <= min_dist) {
      vec3 pos = point;
      vec3 nor = calcNormal(pos);
      vec3  lig = normalize(sun);
      float dif = clamp(dot(nor,lig),0.0,1.0);
      float sha = calcSoftshadow( pos, lig, min_dist, max_dist, 16.0 );
      float amb = 0.5 + 0.5*nor.y;
      float depth = 1. - length(start - pos) / (near_z - far_z);
      pixel.rgb = vec3(0.1647, 0.2235, 0.2824)*amb*r.color + vec3(1.00,0.9,0.80)*dif*sha*r.color; // color with shadows
      //pixel.rgb = vec3(1. - length(start - pos) / (near_z - far_z)); // depth
      //pixel.rgb = vec3(0.05,0.1,0.15)*amb + vec3(1.00,0.9,0.80)*dif*sha*r.color;
      //pixel.rgb = vec3(0.05,0.1,0.15)*amb + r.color*dif*sha;
      break;
    }
    point += direction * r.dist;
  }
  //pixel.rgb = texture(textures[0], vec3((storePos.x / center.x / 2. - center.y)*0.25, (storePos.y / center.y / 2. - center.y)*0.25, time)).rgb;
  //pixel.rgb = vec3(sin(time + storePos.x * 0.1 + storePos.y * 0.1) * 0.5 + 0.5, sin(time * 2 + storePos.y * 0.1) * 0.5 + 0.5, sin(time * 3 + (storePos.y  + storePos.x) * 0.1) * 0.5 + 0.5);
  /*bool cond = false;
  mat4x4 m = transpose(mat4x4(
    1, 0, 0, 1,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  ));
  vec4 vector = vec4(0., 0., 0., 1.);*/
  //bool cond = scene.primitives[0].primitive_type == 0.;
  //pixel.rgb = vec3(float(step_count) / float(steps) == 0 ? 1. : 0.);
  
  
  
  imageStore(destTex, storePos, pixel);
}