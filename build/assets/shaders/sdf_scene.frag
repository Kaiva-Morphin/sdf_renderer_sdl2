#version 430 core

uniform float time;
uniform vec2 texture_size;

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
    int operation_type; // 0 - soft add, 1 - soft subtract, 2 - intersection, 3 - interpolation
    int left_member;
    int right_member;
    int overrides;
    float value;
};

struct PrimitiveScene{
    int size;
    int operations;
    Primitive primitives[32];
    PrimitiveOperation ordered_operations[32];
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

float opSmoothUnion( float d1, float d2, float k )
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

float opSmoothSubtraction( float d1, float d2, float k )
{
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h);
}

float opSmoothIntersection( float d1, float d2, float k )
{
    float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h);
}


uniform sampler3D character_texture;


vec3 in_primitive_space(vec3 point, Primitive primitive){
  mat4x4 transform = -primitive.transform;
  return (transform * (vec4(point - primitive.position.xyz, 1.))).xyz;
}


vec4 SampleScene(vec3 point){
  float new_dist = 0.;
  float mindist = 100.;
  vec3 color = vec3(0., 0., 0.);
  float distances[32];
  vec3 colors[32];
  for (int i=0;i<scene.size;i++) {
    Primitive primitive = scene.primitives[i];
    //vec3 tex_color = vec3(sin(i / 3.14), cos(i / 3.14), sin(i));
    colors[i] = texture(character_texture,  vec3(0.5) - (-primitive.texture_transform * vec4(in_primitive_space(point, primitive), 1.)).xyz).rgb; // 0.1 scale ~ 5. render_units && 1 scale ~ 0.5 render_units
    new_dist = primitive.primitive_type == 0 ?
      sdSphere(in_primitive_space(point, primitive), primitive.rounding)
    :
    primitive.primitive_type == 1 ?
    sdCapsule(
            in_primitive_space(point, primitive),
            -primitive.a.xyz,
            -primitive.b.xyz,
            primitive.rounding
          )
    :
    primitive.primitive_type == 2 ?
    sdBox(in_primitive_space(point, primitive), primitive.a.xyz) - primitive.rounding
    :
    primitive.primitive_type == 3 ?
    sdCappedCylinder(
            in_primitive_space(point, primitive),
            -primitive.a.xyz,
            -primitive.b.xyz,
            primitive.rounding
          )
    :
    udTriangle(
            in_primitive_space(point, primitive),
            -primitive.a.xyz,
            -primitive.b.xyz,
            -primitive.c.xyz
        ) - primitive.rounding;
    distances[i] = new_dist;
  }
  for (int i=0;i<scene.operations;i++) {
    PrimitiveOperation op = scene.ordered_operations[i];
    float left = op.left_member != -1 ? distances[op.left_member] : 100.;
    float right = op.right_member != -1 ? distances[op.right_member] : 100.;
    vec3 left_color = op.left_member != -1 ? colors[op.left_member] : vec3(0., 0., 0.);
    vec3 right_color = op.right_member != -1? colors[op.right_member] : vec3(0., 0., 0.);
    switch (op.operation_type){
      case 0:
        distances[op.overrides] = opSmoothUnion(right, left, op.value);
        float interpolation = 1. - clamp((left - right) / op.value, 0.0, 1.0);
        colors[op.overrides] = mix(right_color, left_color, interpolation);
        /*float mixFactor = clamp((mindist / distances[op.member]), 0.0, 1.0);
        color = mix(color, colors[op.member], mixFactor);*/
        break;
      case 1:
        distances[op.overrides] = opSmoothSubtraction(left, right, op.value);
        break;
      case 2:
        distances[op.overrides] = opSmoothIntersection(right, left, op.value);
        break;
      case 3:
        distances[op.overrides] = mix(left, right, op.value);
        colors[op.overrides] = mix(left_color, right_color, op.value);
    }
  }
  //mindist = opSmoothSubtraction(distances[0], distances[1], 0.1);
  return vec4(colors[scene.operations-1], distances[scene.operations-1]);
}

const float near_z = 12.;
const float far_z = -12.5;
const float max_depth = 1; // local minmaxes
const float min_depth = -1; // local minmaxes
const float min_dist = 0.01;
const float max_dist = abs(near_z) + abs(far_z) + 1;
const int steps = 128;

vec3 calcNormal( in vec3 p ) 
{
    const float eps = 0.001; 
    const vec2 h = vec2(eps,0);
    return normalize( vec3(SampleScene(p+h.xyy).w - SampleScene(p-h.xyy).w,
                           SampleScene(p+h.yxy).w - SampleScene(p-h.yxy).w,
                           SampleScene(p+h.yyx).w - SampleScene(p-h.yyx).w ) );
}

float calcSoftshadow( in vec3 ro, in vec3 rd, float tmin, float tmax, const float k )
{
	float res = 1.0;
    float t = tmin;
    for( int i=0; i<50; i++ )
    {
		float h = SampleScene( ro + rd*t ).w;
        res = min( res, k*h/t );
        t += clamp( h, 0.02, 0.20 );
        if( res<0.005 || t>tmax ) break;
    }
    return clamp( res, 0.0, 1.0 );
}

const float scale_factor = 0.11;

/*

//*     Y
//*     |
//*     * -- X
//*      \
//*        Z

*/

mat4x4 eulerXYZ(float anglex, float angley, float anglez){
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

mat4x4 eulerZYX(float anglex, float angley, float anglez){
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

uniform sampler2D map_depth;
uniform vec2 self_depth_range;
uniform vec4 depth_texture_rect;

float remap(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    float normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

mat4 eye4(){
  return mat4( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

mat4 with_offset(mat4 mat, vec3 offset){
    mat[3] = vec4(offset, mat[3][3]);
    return mat;
}
out vec4 frag_color;
in vec2 fragTexCoord;

mat4 view_mat = mat4x4( 
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0.241, 0.241, 0.5, 0.,
    0., 0., 0, 1.
  );

void main() {
  //view_mat = mat4( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
  //view_mat = eulerXYZ(0, time*150, 0);
  //view_mat = eulerXYZ(0, sin(time)*45 + 20, 0);
  vec2 pixel_pos = fragTexCoord.xy * texture_size; // pixel pos
  vec2 uv_pos = 1 - fragTexCoord;
  vec3 bg_color = vec3(0.0863, 0.1765, 0.2549);
  vec3 light_color =vec3(1.00,0.9,0.80);
  vec4 pixel_color = vec4(bg_color, 0.0);
  vec3 sun = normalize(vec3(0, 1, 0.5)); // sun direction
  vec3 start = (view_mat*vec4((pixel_pos.x - (texture_size.x * 0.5)) * scale_factor, (pixel_pos.y - (texture_size.y * 0.5)) * -scale_factor, near_z, 0)).xyz;
  vec3 point = start;
  vec3 direction = (view_mat*vec4(0., 0., -1., 0.)).xyz;
  
  // torso_
  scene.primitives[0].texture_transform = mat4( 0.01, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1);
  scene.primitives[0].position = vec4(0, 0, -0.2, 0);
  scene.primitives[0].primitive_type = 2;
  scene.primitives[0].a = vec4(0.75, 0.5, 0.75, 0);
  scene.primitives[0].rounding = 0;
  scene.primitives[0].transform = eulerZYX(25, 45, 0);

  

  // torso
  scene.primitives[1].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.01, 0, 0, 0, 0, 0.01, 0, 0, -10, -10, 1);
  scene.primitives[1].position = vec4(0, -1.5, -0.6, 0);
  scene.primitives[1].primitive_type = 0;
  scene.primitives[1].rounding = 0.8;
  scene.primitives[1].transform = eye4();


  // head
  scene.primitives[2].texture_transform = mat4(0.3, 0, 0, 0,0, 0.3, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 1);
  scene.primitives[2].position = vec4(0, 1.7, 0.3, 0);
  scene.primitives[2].primitive_type = 0;
  scene.primitives[2].rounding = 1;
  scene.primitives[2].transform = eye4();

  scene.primitives[3].texture_transform = mat4( 1, 0, 0, 0, 0, 0.2, 0, 0, 0, 0, 1, 0, 0, 0.3, 10, 1) * eulerXYZ(-150, 0, -90);
  scene.primitives[3].position = vec4(0, 2.2, -0.4, 0);
  scene.primitives[3].primitive_type = 3;
  scene.primitives[3].a = vec4(0,  -0.01, 0, 0);
  scene.primitives[3].b = vec4(0, 0.01, 0, 0);
  scene.primitives[3].rounding = 1.2;
  scene.primitives[3].transform = eulerZYX(90, 0, 90);

  
  // rleg
  scene.primitives[4].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 0.01, 0, 0, -0.3, -10, 1);
  scene.primitives[4].position = vec4(0.5, -2.4, -0.5, 0);
  scene.primitives[4].primitive_type = 1;
  scene.primitives[4].a = vec4(0., 0., 0., 0);
  scene.primitives[4].b = vec4(0., -1., 0., 0);
  scene.primitives[4].rounding = 0.4;
  scene.primitives[4].transform = eye4();

  // rleg_
  scene.primitives[5].texture_transform = mat4( 0.01, 0, 0, 0, 0, -0.1, 0, 0, 0, 0, 0.01, 0, 0, 0.2, 10, 1);
  scene.primitives[5].position = vec4(0.5, -3.5, -0.5, 0);
  scene.primitives[5].primitive_type = 1;
  scene.primitives[5].a = vec4(0., 0., 0.1, 0);
  scene.primitives[5].b = vec4(0., -0.75, -0., 0);
  scene.primitives[5].rounding = 0.6;
  scene.primitives[5].transform = eye4();

  // lleg
  scene.primitives[6].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 0.01, 0, 0, -0.3, -10, 1);
  scene.primitives[6].position = vec4(-0.5, -2.4, -0.5, 0);
  scene.primitives[6].primitive_type = 1;
  scene.primitives[6].a = vec4(0., 0., 0., 0);
  scene.primitives[6].b = vec4(0., -1., 0., 0);
  scene.primitives[6].rounding = 0.4;
  scene.primitives[6].transform = eye4();

  // lleg_
  scene.primitives[7].texture_transform = mat4( 0.01, 0, 0, 0, 0, -0.1, 0, 0, 0, 0, 0.01, 0, 0, 0.2, 10, 1);
  scene.primitives[7].position = vec4(-0.5, -3.5, -0.5, 0);
  scene.primitives[7].primitive_type = 1;
  scene.primitives[7].a = vec4(0., 0., 0., 0);
  scene.primitives[7].b = vec4(0., -0.75, -0., 0);
  scene.primitives[7].rounding = 0.6;
  scene.primitives[7].transform = eye4();

  // rarm
  scene.primitives[8].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.2, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1)  * eulerXYZ(0, 0, 135);
  scene.primitives[8].position = vec4(0.9, 0, -0.25, 0);
  scene.primitives[8].primitive_type = 0;
  scene.primitives[8].rounding = 0.7;
  scene.primitives[8].transform = eye4();

  scene.primitives[9].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.01, 0, 0, 0, 0, 0.01, 0, 0, -0.2, -10, 1);
  scene.primitives[9].position = vec4(1.2, 0, -0.25, 0);
  scene.primitives[9].primitive_type = 1;
  scene.primitives[9].a = vec4(0., 0., 0., 0);
  scene.primitives[9].b = vec4(0., -1.5, 0.0, 0);
  scene.primitives[9].rounding = 0.35;
  scene.primitives[9].transform = eye4();

  // rarm_
  scene.primitives[10].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.3, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1);
  scene.primitives[10].position = vec4(1.2, -1.5, -0.25, 0);
  scene.primitives[10].primitive_type = 1;
  scene.primitives[10].a = vec4(0., 0., 0., 0);
  scene.primitives[10].b = vec4(0., -1, 0.0, 0);
  scene.primitives[10].rounding = 0.45;
  scene.primitives[10].transform = eye4();

  // larm
  scene.primitives[11].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.2, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1)  * eulerXYZ(0, 0, 135);
  scene.primitives[11].position = vec4(-0.9, 0, -0.25, 0);
  scene.primitives[11].primitive_type = 0;
  scene.primitives[11].rounding = 0.7;
  scene.primitives[11].transform = eye4();

  scene.primitives[12].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.01, 0, 0, 0, 0, 0.01, 0, 0, -0.2, -10, 1);
  scene.primitives[12].position = vec4(-1.2, 0, -0.25, 0);
  scene.primitives[12].primitive_type = 1;
  scene.primitives[12].a = vec4(0., 0., 0., 0);
  scene.primitives[12].b = vec4(0., -1.5, 0.0, 0);
  scene.primitives[12].rounding = 0.35;
  scene.primitives[12].transform = eye4();

  // larm_
  scene.primitives[13].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.3, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1);
  scene.primitives[13].position = vec4(-1.2, -1.5, -0.25, 0);
  scene.primitives[13].primitive_type = 1;
  scene.primitives[13].a = vec4(0., 0., 0., 0);
  scene.primitives[13].b = vec4(0., -1, 0.0, 0);
  scene.primitives[13].rounding = 0.45;
  scene.primitives[13].transform = eye4();

  scene.size = 14;
  scene.operations = 14;
  scene.ordered_operations[0] = PrimitiveOperation(0, 0, 1, 1, 2);
  scene.ordered_operations[1] = PrimitiveOperation(0, 3, 2, 2, 0.8);
  scene.ordered_operations[2] = PrimitiveOperation(0, 1, 2, 3, 0);
  scene.ordered_operations[3] = PrimitiveOperation(0, 2, 3, 3, 0.8);
  scene.ordered_operations[4] = PrimitiveOperation(0, 3, 4, 4, 0);
  scene.ordered_operations[5] = PrimitiveOperation(0, 4, 5, 5, 0);
  scene.ordered_operations[6] = PrimitiveOperation(0, 5, 6, 6, 0);
  scene.ordered_operations[7] = PrimitiveOperation(0, 6, 7, 7, 0);
  scene.ordered_operations[8] = PrimitiveOperation(0, 7, 8, 8, 0);
  scene.ordered_operations[9] = PrimitiveOperation(0, 8, 9, 9, 0);
  scene.ordered_operations[10] = PrimitiveOperation(0, 9, 10, 10, 0);
  scene.ordered_operations[11] = PrimitiveOperation(0, 10, 11, 11, 0);
  scene.ordered_operations[12] = PrimitiveOperation(0, 11, 12, 12, 0);
  scene.ordered_operations[13] = PrimitiveOperation(0, 12, 13, 13, 0);


  //scene.primitives[0].transform[3] = vec4(0, 2, 0, 1);
  //scene.primitives[2].position.y = 4;
  //cene.primitives[2].transform[3] = vec4(0, -2, 0, 1);
  /*scene.primitives[0].position.y = -2;
  scene.primitives[0].a.z = 5;
  scene.primitives[0].transform = eulerZYX(0, time*100, 0);
  scene.primitives[0].texture_transform = mat4(
    0.1, 0, 0, 0,
    0, 0.1, 0, 0, 
    0, 0, 0.1, 0,
    0, 0, 0, 1
  );
  scene.primitives[0].transform[3] = vec4(0, -1, 0, 1);
  */

  //scene.primitives[0].transform = eulerXYZ(sin(time) * 360, 0, 0);

  //float wrapped_time = float(int(time * 200) % (12 * 100)) / 100;
  //float a = clamp(wrapped_time,      0, 1);
  //float b = clamp(wrapped_time - 2,  0, 1);
  //float c = clamp(wrapped_time - 4,  0, 1);
  //float d = clamp(wrapped_time - 6,  0, 1);
  //float e = clamp(wrapped_time - 8,  0, 1);
  //float f = clamp(wrapped_time - 10, 0, 1);

  //scene.primitives[4].transform = with_offset(eye4(), vec3(sin(time*15)*0.25 + 0.75 , 0, 0)) * eulerXYZ(0, time * 400, time * 40);




  vec2 relative_uv;
  relative_uv.x = depth_texture_rect.x + uv_pos.x * (depth_texture_rect.z - depth_texture_rect.x) + (0.1 / texture_size.x);
  relative_uv.y = 1 - (depth_texture_rect.y + (1-uv_pos.y) * (depth_texture_rect.w - depth_texture_rect.y) - (0.25 / texture_size.y));
  float texture_depth = texture(map_depth, relative_uv).r;
  for (int i=0;i<steps;i++) {
    vec4 r = SampleScene(point);
    float dist = r.w;
    vec3 color = r.rgb;
    if (dist > max_dist || point.z <= far_z){
      break;
    }
    if (dist <= min_dist) {
      vec3 pos = point;
      vec3 nor = calcNormal(pos);
      vec3 lig = normalize(sun);
      float dif = clamp(dot(nor,lig),0.0,1.0);
      float sha = calcSoftshadow( pos, lig, min_dist, max_dist, 16.0 );
      float amb = 0.9 + 0.1 * nor.y;
      float depth = remap((near_z - start.z - pos.z) * -1, min_depth, max_depth, self_depth_range.x, self_depth_range.y);
      depth = remap(pos.z, min_depth, max_depth, self_depth_range.x, self_depth_range.y);
      vec3 result_color = bg_color*amb*color + light_color*dif*sha*color;
      //pixel_color.rgb   // color with shadows
      //result_color = vec3(depth);
      if (depth > texture_depth || texture_depth==0){
        pixel_color.rgb = result_color;  // color with shadows
        pixel_color.a = 1;
      } else {
        pixel_color.rgb = result_color * 0.5;
        //pixel_color.a = 0;
        pixel_color.a = ((int(sin(pixel_pos.x * (cos(time) * 0.5 + 1)) + pixel_pos.y + pixel_pos.x * -0.25 + time * 30) % 8) < 6)?0.5:0;
      }
      //pixel_color.rgb = vec3(depth);
      break;
    }
    point += direction * dist;
  }
  //pixel_color.a = 1;
  if (pixel_color.a == 0){
    //pixel_color.a = 1;
    //pixel_color.rgb = vec3(texture_depth);
  }
  //pixel_color.rgb = vec3(texture_depth);
  //pixel_color.a = 1;
  //pixel_color.rgb = vec3(relative_uv.x, relative_uv.y, 0) * texture_depth;

  
  //pixel_color.rgb = vec3(depth_texture_rect.z, depth_texture_rect.w, 0);
  //pixel_color.rgb = vec3(relative_uv.x, relative_uv.y, 0);
  //pixel_color.a = 0.5;
  /*
  pixel_color.rgb = vec3(uv_pos.y * (depth_texture_rect.w - depth_texture_rect.y) + (0.25 / texture_size.y) + depth_texture_rect.y);
  pixel_color.rgb = vec3(relative_uv.x);
  pixel_color.rgb = vec3((depth_texture_rect.y) * -1 + uv_pos.y * (depth_texture_rect.w - depth_texture_rect.y) + (0.25 / texture_size.y));
  pixel_color.rgb = vec3(depth_texture_rect.y + uv_pos.y * (depth_texture_rect.w - depth_texture_rect.y) + (0.25 / texture_size.y));
  */
  //pixel_color.rgb = vec3(0.5);
  //pixel_color.r = (((int(pixel_pos.x + pixel_pos.y) % 2) == 0)?0.2:0);
  //pixel_color = vec4(1, 0, 0, 1);
  frag_color = pixel_color;
}