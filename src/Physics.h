#include "Game.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include "cmath"
#include "Functions.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>
#include <variant>
#include <vector>
using namespace std;

#include <chrono>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <algorithm>

#include <thread>

#define ALMOST_ZERO 0.00001

#define TYPE_FIXED 0
#define TYPE_MOVING 1
#define TYPE_RIGID 2

#define SHAPE_CAPSULE 0 // always rigid
#define SHAPE_LINE 1 //  moving or fixed

struct alignas(16) PhysicsPrimitive{
    vec4 position = vec4(0.);
    vec4 velocity = vec4(0.);
    vec4 a = vec4(0.);
    vec4 b = vec4(0.);
    vec4 c = vec4(0.);
    vec4 normal = vec4(0.);
    float rounding = 0.;
    float bounciness = 0.1;
    float mass = 1.;
    int type = TYPE_FIXED;
    int shape = SHAPE_CAPSULE;
    bool y_slopes = false;
    float friction = 0.;
};

class PhysicsSolver{

    // shader
    private:
    GLuint program = glCreateProgram();
    GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    GLuint input_buffer = 0;
    GLuint output_buffer = 0;
    string file_path;
    string current_src;
    int ccd_steps = 2; // two steps for 2d, 3 for 3d))
    string read_shader(string path=""){
        if (path=="") path = file_path;
        ifstream fileStream(path);
        if (!fileStream.is_open()) {
            cerr << "ERROR! Cant open file!" << endl;
            return "";
        }
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }

    void compile(){
        const GLchar* src = current_src.c_str();
        glShaderSource(compute_shader, 1, &src, NULL);
        glCompileShader(compute_shader);
        GLint success;
        glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(compute_shader, 512, NULL, infoLog);
            std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
            return;
        }
        glAttachShader(program, compute_shader);
        glLinkProgram(program);
        glDeleteShader(compute_shader);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
            return;
        }
        glGenBuffers(1, &input_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input_buffer);
        glGenBuffers(1, &output_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output_buffer);
    }
    void set_1f(const char* name, float value){
        glUniform1f(glGetUniformLocation(program, name), value);
    }
    void set_1i(const char* name, int value){
        glUniform1i(glGetUniformLocation(program, name), value);
    }
    void set_3f(const char* name, vec3 value){
        glUniform3f(glGetUniformLocation(program, name), value.x, value.y, value.z);
    }
    public:
    void check_file_updates(){
        string new_src = read_shader();
        if (new_src != current_src){
            current_src = new_src;
            compile();
        }
    }
    PhysicsSolver(string compute_shader_path){
        file_path = compute_shader_path;
    }
    void init(){
        current_src = read_shader();
        compile();
    }

    void draw_line(vec2 pa, vec2 pb, vec3 color){
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        vec2 ld = pa;
        vec2 ru = pb;
        float a = remap(ld.x, -half_screen.x, half_screen.x, -1, 1);
        float b = remap(ld.y, -half_screen.y, half_screen.y, -1, 1);
        float c = remap(ru.x, -half_screen.x, half_screen.x, -1, 1);
        float d = remap(ru.y, -half_screen.y, half_screen.y, -1, 1);


        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(a, b);
            glVertex2f(c, d);
            glColor3f(1., 1., 1.);
        glEnd();
    }

    void draw_box(vec2 pos, vec2 size, vec3 color){
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        vec2 ld = pos - size * 0.5f;
        vec2 ru = pos + size * 0.5f;
        float a = remap(ld.x, -half_screen.x, half_screen.x, -1, 1);
        float b = remap(ld.y, -half_screen.y, half_screen.y, -1, 1);
        float c = remap(ru.x, -half_screen.x, half_screen.x, -1, 1);
        float d = remap(ru.y, -half_screen.y, half_screen.y, -1, 1);


        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(a, b);
            glVertex2f(a, d);
            glVertex2f(a, d);
            glVertex2f(c, d);
            glVertex2f(c, d);
            glVertex2f(c, b);
            glVertex2f(c, b);
            glVertex2f(a, b);
            glColor3f(1., 1., 1.);
        glEnd();

    }
    void draw_capsule(vec2 pos, vec2 size, vec3 color){
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        vec2 ld = pos - size * vec2(1., 0.5);
        vec2 ru = pos + size * vec2(1., 0.5);

        vec2 uv_pos = remap_vec2(pos, -half_screen, half_screen, vec2(-1), vec2(1));
        vec2 radius = remap_vec2(vec2(size.x), -half_screen, half_screen, vec2(-1), vec2(1));
        float height = remap(size.y, -half_screen.y, half_screen.y, -1, 1);

        float lx = remap(ld.x, -half_screen.x, half_screen.x, -1, 1);
        float ly = remap(ld.y, -half_screen.y, half_screen.y, -1, 1);
        float rx = remap(ru.x, -half_screen.x, half_screen.x, -1, 1);
        float ry = remap(ru.y, -half_screen.y, half_screen.y, -1, 1);

        vec2 a = vec2(lx, ly);
        vec2 b = vec2(rx, ry);
        
        float sin30 = 0.5;
        float cos30 = 0.866;
        float sin60 = 0.866;
        float cos60 = 0.5;

        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(a.x, a.y); // ld
            glVertex2f(a.x, b.y); // lu

            glVertex2f(a.x, b.y); // lu
            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);

            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);
            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);

            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);
            glVertex2f(uv_pos.x, uv_pos.y + height * 0.5f + radius.y);

            glVertex2f(uv_pos.x, uv_pos.y + height * 0.5f + radius.y);
            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);

            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);
            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);

            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);
            glVertex2f(b.x, b.y); // ru

            glVertex2f(b.x, b.y); // ru
            glVertex2f(b.x, a.y); // rd

            glVertex2f(b.x, a.y); // rd
            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);

            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);
            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);

            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);
            glVertex2f(uv_pos.x, uv_pos.y - height * 0.5f - radius.y);

            glVertex2f(uv_pos.x, uv_pos.y - height * 0.5f - radius.y);
            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);


            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);
            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);

            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);
            glVertex2f(a.x, a.y); // ld

            glColor3f(1., 1., 1.);
        glEnd();
    }
    void draw_pyramid(vec2 pos, vec2 plane, vec2 vertex, vec3 color){
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        vec2 plane_left = pos - vec2{plane.x * 0.5f, -plane.y};
        vec2 plane_right = pos + vec2{plane.x * 0.5f, plane.y};
        vec2 vertex_offset = pos + vertex;
        vec2 uv_plane_l = remap_vec2(plane_left, -half_screen, half_screen, vec2(-1), vec2(1));
        vec2 uv_plane_r = remap_vec2(plane_right, -half_screen, half_screen, vec2(-1), vec2(1));
        vec2 uv_vert = remap_vec2(vertex_offset, -half_screen, half_screen, vec2(-1), vec2(1));
        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(uv_plane_l.x, uv_plane_l.y);
            glVertex2f(uv_plane_r.x, uv_plane_r.y);
            glVertex2f(uv_plane_r.x, uv_plane_r.y);
            glVertex2f(uv_vert.x, uv_vert.y);
            glVertex2f(uv_vert.x, uv_vert.y);
            glVertex2f(uv_plane_l.x, uv_plane_l.y);
            glColor3f(1., 1., 1.);
        glEnd();
    }
    vec3 get_color(int type){
        switch (type){
            case TYPE_FIXED:
                return vec3(1., 0., 0.);
                break;
            case TYPE_MOVING:
                return vec3(1., 0., 1.);
                break;
            case TYPE_RIGID:
                return vec3(0., 0., 1.);
                break;
        }
        return vec3(0.);
    }
    void draw_point(vec2 pos, vec3 color){
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        vec2 pos_uv = remap_vec2(pos, -half_screen, half_screen, {-1, -1}, {1, 1});
        glBegin(GL_POINTS);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(pos_uv.x, pos_uv.y);
            glColor3f(1., 1., 1.);
        glEnd();
    }

    

    void draw_arrow(vec2 from, vec2 to, vec3 color){
        vec2 half_screen = vec2(game->screen_pixel_size) * 0.5f;
        float a = remap(from.x, -half_screen.x, half_screen.x, -1, 1);
        float b = remap(from.y, -half_screen.y, half_screen.y, -1, 1);
        float c = remap(to.x, -half_screen.x, half_screen.x, -1, 1);
        float d = remap(to.y, -half_screen.y, half_screen.y, -1, 1);


        glBegin(GL_LINES);
            glColor3f(0, 0, 0);
            glVertex2f(a, b);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(c, d);
            glColor3f(1., 1., 1.);
        glEnd();
    }
    void draw(){
        for (PhysicsPrimitive* object: objects){
            vec3 color = get_color(object->type);
            vec2 pos = vec2{object->position.x, object->position.y};
            
            switch (object->shape){
                case SHAPE_CAPSULE:
                    draw_capsule(pos, vec2(object->rounding, object->a.x), color);
                    draw_point(pos, color);
                    break;
                case SHAPE_LINE:
                    draw_line(pos+vec2(object->a), pos+vec2(object->b), color);
                    vec2 center = (pos+vec2(object->a) + pos+vec2(object->b)) / 2.0f;
                    vec4 norm = normal_of_line(object->a, object->b);
                    draw_arrow(center, center+vec2(norm)*10.0f, color);
                    break;
            }
        }
    }
    // constructors
    PhysicsPrimitive box(vec3 size=vec3(10.)){return PhysicsPrimitive{};}// todo: bundle
    PhysicsPrimitive line(vec3 a, vec3 b){
        PhysicsPrimitive p;
        p.a = vec4(a, 0);
        p.b = vec4(b, 0);
        p.shape = SHAPE_LINE;
        p.normal = normal_of_line(p.a, p.b);
        return p;
    }// todo: bundle

    PhysicsPrimitive capsule(float height = 10., float radius = 10){
        PhysicsPrimitive p;
        p.a = vec4(height);
        p.rounding = radius;
        p.shape = SHAPE_CAPSULE;
        return p;
    }
    // todo: bundle
    PhysicsPrimitive pyramid( // !WARN! CENTER MUST BE INSIDE PRIMITIVE! 
        vec3 base, // xz is plane size, y is y offset 
        vec3 vert // vertex offset
    ){return PhysicsPrimitive{};}

    void push(PhysicsPrimitive* p){
        if (p->shape == SHAPE_LINE) objects.push_back(p);
        else { // ordering.
            for (int i = 0; i < objects.size(); i++) {
                if (objects[i]->shape == SHAPE_LINE){
                    objects.insert(objects.begin() + i, p);
                    return;
                }
            }
            objects.push_back(p);
        }
    }

    vec4 get_aabb_max(PhysicsPrimitive p){ // todo: switch case
        return p.position + vec4(p.rounding, p.rounding, p.rounding, 0.) + vec4(0., p.a.x * 0.5, 0., 0.);
    }

    vec4 get_aabb_min(PhysicsPrimitive p){ // todo: switch case
        return p.position - vec4(p.rounding, p.rounding, p.rounding, 0.) - vec4(0., p.a.x * 0.5, 0., 0.);
    }

    bool intersects(PhysicsPrimitive p1, PhysicsPrimitive p2){ // todo: make for 3d 
        vec4 aabb_min_1 = get_aabb_min(p1);
        vec4 aabb_max_1 = get_aabb_max(p1);
        vec4 aabb_min_2 = get_aabb_min(p2);
        vec4 aabb_max_2 = get_aabb_max(p2);
        return aabb_min_1.x <= aabb_max_2.x && aabb_min_1.y <= aabb_max_2.y && aabb_min_1.z <= aabb_max_2.z &&
                aabb_max_1.x >= aabb_min_2.x && aabb_max_1.y >= aabb_min_2.y && aabb_max_1.z >= aabb_min_2.z;
    }

    bool is_point_in_AABB(PhysicsPrimitive p1, vec4 p2){ // todo: make for 3d
        vec4 aabb_min_1 = get_aabb_min(p1);
        vec4 aabb_max_1 = get_aabb_max(p1);
        return aabb_min_1.x <= p2.x && aabb_min_1.y <= p2.y && aabb_min_1.z <= p2.z &&
                aabb_max_1.x >= p2.x && aabb_max_1.y >= p2.y && aabb_max_1.z >= p2.z;
    }

    vec4 apply_rounding(vec4 position, vec4 destination, float rounding){
        if (rounding == 0.){return position;};
        if (destination == position){return position;};
        vec4 vector = destination - position;
        return position + normalize(vector) * rounding;
    };
    vec4 apply_inv_rounding(vec4 position, vec4 destination, float rounding){
        if (rounding == 0.){return position;};
        if (destination == position){return position;};
        vec4 vector = destination - position;
        return position - normalize(vector) * rounding;
    };

    float length_squared(vec2 vec){
        return vec.x * vec.x + vec.y * vec.y;
    }

    float length_squared(vec3 vec){
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }
    
    float length_squared(vec4 vec){
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
    }

    float clamp(float v, float minv, float maxv){
        return glm::min(glm::max(v, minv), maxv);
    }

    vec4 normal_of_line(vec4 a, vec4 b){
        vec4 vec = a - b;
        return normalize(vec4(-vec.y, vec.x, 0, 0));
    }

    vec4 closest_point(PhysicsPrimitive* p1, vec4 point){ // todo: make for 3d, split to shape-specific
        switch (p1->shape){
            case SHAPE_CAPSULE:{
                float y = point.y;
                if (p1->position.x == point.x) {
                    if (point.y < p1->position.y) return p1->position - vec4(0., p1->a.x*0.5f, 0., 0.);
                    else                          return p1->position + vec4(0., p1->a.x*0.5f, 0., 0.);
                } else {
                    y = glm::min(y, p1->position.y+p1->a.x*0.5f);
                    y = glm::max(y, p1->position.y-p1->a.x*0.5f);
                }
                return vec4(p1->position.x, y, p1->position.z, p1->position.w);
                break;
            }
        }
        return vec4(0);
    }

    vec4 inv_closest_capsule_point(PhysicsPrimitive p1, vec4 point){ // todo: make for 3d
        float y = point.y;
        if (p1.position.x == point.x) {
            if (point.y < p1.position.y) return p1.position + vec4(0., p1.a.x*0.5f, 0., 0.);
            else                         return p1.position - vec4(0., p1.a.x*0.5f, 0., 0.);
        } else {
            y = glm::max(y, p1.position.y-p1.a.x*0.5f);
            y = glm::min(y, p1.position.y+p1.a.x*0.5f);
        }
        return vec4(p1.position.x, y, p1.position.z, p1.position.w);
    }

    vec4 closest_point_on_capped_line(vec4 point, vec4 a, vec4 b){
        vec4 AB = a - b;
        vec4 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        t = clamp(t, 0.0, 1.0);
        return b + t * AB;
    }

    vec4 closest_vertex_on_capped_line(vec4 point, vec4 a, vec4 b){
        vec4 AB = a - b;
        vec4 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        if (t > 0.5) t = 1;
        else t = 0;
        return b + t * AB;
    }

    vec3 closest_point_on_capped_line(vec3 point, vec3 a, vec3 b){
        vec3 AB = a - b;
        vec3 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        t = clamp(t, 0.0, 1.0);
        return b + t * AB;
    }

    vec4 closest_point_on_line(vec4 point, vec4 a, vec4 b){
        vec4 AB = a - b;
        vec4 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        return b + t * AB;
    }

    vec3 closest_point_on_line(vec3 point, vec3 a, vec3 b){
        vec3 AB = a - b;
        vec3 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        return b + t * AB;
    }

    float cross_product(vec2 v1, vec2 v2){
        return v1.x * v2.y - v1.y * v2.x;
    }

    vec4 capsule_vs_line_closest_point(PhysicsPrimitive p1, vec4 a, vec4 b, vec4 center){
        vec4 upper = p1.position;
        upper.y += p1.a.x * 0.5;
        vec4 lower = p1.position;
        lower.y -= p1.a.x * 0.5;
        bool same_side = on_same_side(a, b, p1.position, center);
        if (a.x == b.x || (a.y == b.y && (p1.position.x < glm::min(a.x, b.x) || p1.position.x > glm::max(a.x, b.x)))) { // if x line and x center is out of bounds
            float y = clamp(a.y, lower.y, upper.y);
            return p1.position * vec4(1, 0, 1, 1) + vec4(0, y, 0, 0);
        } else {
            float k = (a.y - b.y) / (a.x - b.x);
            float line_y = b.y + k * (p1.position.x - b.x);
            if (same_side) line_y = b.y - k * (p1.position.x - b.x);
            line_y = glm::min(line_y, glm::max(b.y, a.y));
            line_y = glm::max(line_y, glm::min(b.y, a.y));
            // clamp(line_y, p1.position.y - p1.a.x * 0.5, p1.position.y + p1.a.x * 0.5
            line_y = abs(lower.y - line_y) < abs(upper.y - line_y)?same_side?upper.y:lower.y:same_side?lower.y:upper.y;
            return p1.position * vec4(1, 0, 1, 1) + vec4(0, line_y, 0, 0);
        }
    }


    bool on_same_side(vec2 a, vec2 b, vec2 center, vec2 point){
        float A = a.y - b.y;
        float B = b.x - a.x;
        float C = a.x * b.y - b.x * a.y;
        float r1 = A * center.x + B * center.y + C;
        float r2 = A * point.x + B * point.y + C;
        return (r1 * r2) > 0;
    }

    void lines2(BDFAtlas* font_atlas, vec2 cursor){
        static float time = 0;
        time += 0.001;
        vec3 a1 = { 0, 0, 0};
        vec3 a2 = vec3(cursor, 0);
        vec3 b1 = { 0, 0, 0};
        vec3 b2 = { 0, -50, 0};

        draw_line(a1, a2, {1, 0, 0});
        draw_line(b1, b2, {0, 1, 0});

        vec3 result = inv_projection(b2, normalize(a2));

        draw_line(a1, result, {0, 0, 1});


    }

    void lines(BDFAtlas* font_atlas){
        static float time = 0;
        time += 0.001;
        vec3 a1 = { sin(time) * 60, 0, 0};
        vec3 a2 = { 30 + sin(time) * 60, cos(time * 4) * 30, 0};
        vec3 b1 = { 30, -50, 0};
        vec3 b2 = {-30, -50, 0};
        draw_line(a1, a2, {1, 0, 0});
        draw_line(b1, b2, {0, 1, 0});
        vec3 end_a, end_b;
        tie(end_a, end_b) = closest_points_between_line_segments_3d(a1, a2, b1, b2);
        
        vec3 B = closest_point_on_line(a1, b1, b2);
        vec3 AB = B - a1;
        vec3 result = a1 + inv_projection(AB, end_b - end_a);


        
        draw_line(result, a1, {1, 1, 1});

        draw_line(end_b, end_a, {0, 1, 1});




        //draw_capsule(a1 + result - b1, {10, 0}, game->screen_pixel_size, {0, 0, 1});
        //draw_line(a, b, game->screen_pixel_size, {0, 0, 1});
    }

    vec3 inv_projection(vec3 a, vec3 dir) {
        float target_len = length(a);
        float cos_ = (dot(a, dir) / (length(dir) * target_len));
        return target_len / cos_ * normalize(dir);
    }

    vec4 inv_projection(vec4 a, vec4 dir) {
        float target_len = length(a);
        float cos_ = (dot(a, dir) / (length(dir) * target_len));
        return target_len / cos_ * normalize(dir);
    }

    float angle(vec4 a, vec4 b){
        return acos(dot(a, b) / (length(a) * length(b)));
    }

    struct InspectionResult{
        vec4 start;
        vec4 end;
        vec4 point;
        float offset;
        vec4 line_point;
        float start_dist_sqared;
    };

    std::tuple<vec4, vec4> closest_points_between_line_segments_3d(vec3 a0, vec3 a1, vec3 b0, vec3 b1){
        if (a0 == a1) {return std::make_tuple(vec4(a0, 0), vec4(closest_point_on_capped_line(a0, b0, b1), 0));}
        if (b0 == b1) {return std::make_tuple(vec4(closest_point_on_capped_line(b0, a0, a1), 0), vec4(b0, 0));}
        vec3 A = a1 - a0;
        vec3 B = b1 - b0;
        float magA = length(A);
        float magB = length(B);
        vec3 _A = A / magA;
        vec3 _B = B / magB;
        vec3 crs = cross(_A, _B);
        float denom = length(crs); 
        denom *= denom;
        if (denom == 0){
            float d0 = dot(_A, (b0-a0));
            float d1 = dot(_A, (b1-a0));
            if ((d0 <= 0) && (0 >= d1)){
                if (abs(d0) < abs(d1)){
                    return std::make_tuple(vec4(a0, 0), vec4(b0, 0));
                } else {
                    return std::make_tuple(vec4(a0, 0), vec4(b1, 0));
                }
            } else if ((d0 >= magA) && (magA <= d1)) {
                if (abs(d0) < abs(d1)){
                    return std::make_tuple(vec4(a1, 0), vec4(b0, 0));
                } else {
                    return std::make_tuple(vec4(a1, 0), vec4(b1, 0));
                }
            }
            // parallel
            vec3 ra = closest_point_on_capped_line(a0, b0, b1);
            vec3 rb =  a0;
            float dist = length_squared(ra - rb);
            vec3 nra = closest_point_on_capped_line(a1, b0, b1);
            vec3 nrb = a1;
            float ndist = length_squared(nra - nrb);
            if (ndist < dist){
                ra = nra;
                rb = nrb;
                dist = ndist;
            }
            nra = closest_point_on_capped_line(b0, a0, a1);
            nrb = b0;
            ndist = length_squared(nra - nrb);
            if (ndist < dist){
                ra = nra;
                rb = nrb;
                dist = ndist;
            }
            nra = closest_point_on_capped_line(b1, a0, a1);
            nrb = b1;
            ndist = length_squared(nra - nrb);
            if (ndist < dist){
                ra = nra;
                rb = nrb;
                dist = ndist;
            }
            return std::make_tuple(vec4(ra, 0), vec4(rb, 0));
        }
        vec3 t = b0 - a0;
        float detA = determinant(mat3(t, _B, crs));
        float detB = determinant(mat3(t, _A, crs));
        
        float t0 = detA/denom;
        float t1 = detB/denom;
        vec3 pA = a0 + (_A * t0);
        vec3 pB = b0 + (_B * t1);
        if (t0 < 0)  pA = a0;
        else if (t0 > magA)  pA = a1;
        if (t1 < 0)  pB = b0;
        else if (t1 > magB)  pB = b1;

        if ((t0 < 0) || (t0 > magA)) {
            float _dot = dot(_B, (pA-b0));
            if (_dot < 0) _dot = 0;
            else if (_dot > magB) _dot = magB;
            pB = b0 + (_B * _dot);
        }
        if ((t1 < 0) || (t1 > magB)) {
            float _dot = dot(_A, (pB-a0));
            if (_dot < 0) _dot = 0;
            else if (_dot > magA) _dot = magA;
            pA = a0 + (_A * _dot);
        }
        return std::make_tuple(vec4(pA, 0), vec4(pB, 0));
    }


    bool on_same_side_and_line(vec2 a, vec2 b, vec2 center, vec2 point){
        vec2 AB = a - b;
        vec2 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        return !((t < 0) || (t > 1)) && on_same_side(a, b, center, point);
    }

    bool is_point_on_capped_line(vec2 point, vec2 a, vec2 b){
        vec2 AB = a - b;
        vec2 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        return !((t < 0) || (t > 1));
    }

    bool is_point_on_capped_line(vec4 point, vec4 a, vec4 b){
        vec4 AB = a - b;
        vec4 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        return !((t < 0) || (t > 1));
    }

    bool is_point_on_capped_line_vertex(vec4 point, vec4 a, vec4 b){
        vec4 AB = a - b;
        vec4 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        return ((t == 0) || (t == 1));
    }

    vec4 binsearch_step(vec4 start, vec4 end, vec4 vertex1, vec4 vertex2, vec4 line_normal, float radius, float height){
        InspectionResult res = closest_capsulecast_vs_line(height * 0.5f, start, end, vertex1, vertex2);
        //closest_capsulecast_vs_line(float half_height, vec4 start, vec4 end, vec4 vertex1, vec4 vertex2)
        vec4 latest_uncollide = vec4(0);
        float distance_sqared = length_squared(res.point - res.line_point);
        float radius_squared = (radius * radius);
        vec4 translation_point, line_point;
        tie(translation_point, line_point) = closest_points_between_line_segments_3d(start+vec4(0, height*0.5f, 0, 0), start-vec4(0, height*0.5f, 0, 0), vertex1, vertex2);

        if (length_squared(translation_point - line_point) > radius_squared){
            latest_uncollide = start;
        } else {
            return end;
        }
        vec4 left = start;
        vec4 right = end;
        if (distance_sqared < radius_squared){ // collision on line
            for (int i = 0; i < 256; i++){
                vec4 center = (left + right) / 2.0f;
                InspectionResult res = closest_capsulecast_vs_line(height * 0.5f, left, center, vertex1, vertex2);
                float l = length_squared(res.point - res.line_point);
                if (l <= radius_squared){
                    latest_uncollide = center;
                    if ((radius_squared - l) < 1e-7f) {/*printf("iter");*/return center;}
                    right = center;
                } else {
                    left = center;
                }
                //draw_capsule(center, {4, 0}, {1, 1, 1});
            }
            
        } else return end;
        return latest_uncollide;
    }

    vec4 math_step(vec4 start, vec4 end, vec4 vertex1, vec4 vertex2, vec4 line_normal, float radius){
        vec4 translation_point, line_point;
        vec4 local_starting_point = start;
        tie(translation_point, line_point) = closest_points_between_line_segments_3d(start, end, vertex1, vertex2);

        float distance_to_travel = length(end - local_starting_point);

        float endpoint_distance_sqared = length_squared(translation_point - line_point);
        float radius_squared = (radius * radius);

        bool endpoint_collision = endpoint_distance_sqared < radius_squared;
        bool startpoint_collision = length(local_starting_point - closest_point_on_capped_line(local_starting_point, vertex1, vertex2)) < radius;

        

        
        if ((endpoint_collision && !startpoint_collision) && dot(end-local_starting_point, line_normal) < 0){ // check normals
            vec4 B = closest_point_on_line(local_starting_point, vertex1, vertex2);
            vec4 AB = B - local_starting_point;
            vec4 dir = (endpoint_distance_sqared < ALMOST_ZERO) ? normal_of_line(local_starting_point, line_point) : normalize(line_point - translation_point);
            vec4 result = inv_projection(AB, dir);
            vec4 inv_proj_radius = inv_projection(line_normal* radius, dir);
            float projected_radius = length(inv_proj_radius);
            float startpoint_dist = length(result);
            float endpoint_dist = sqrt(endpoint_distance_sqared);
            float point_eq_radius_u = remap(projected_radius, startpoint_dist, endpoint_dist, 0, 1);
            vec4 point_eq_radius = local_starting_point + (translation_point - local_starting_point) * point_eq_radius_u;
            if (dot(vertex1-vertex2, local_starting_point - end) == 0){ // parrallel wrapper
                vec4 temp = closest_point_on_capped_line(local_starting_point, vertex1, vertex2); // || closest_point_on_line ?
                point_eq_radius = normalize(local_starting_point-temp)*radius + temp;
            }
            //vec3 color = (is_point_on_capped_line(point_eq_radius, vertex1, vertex2))?vec3{0, 1, 0}:vec3{1, 0, 0};
            // if point out of line bounds (intersection with point)
            vec4 nearest_vertex = closest_vertex_on_capped_line(local_starting_point, vertex1, vertex2);
            vec4 nearest_vertex_point = closest_point_on_line(nearest_vertex, local_starting_point, end);
            float x = sqrt(radius*radius - length_squared(nearest_vertex_point - nearest_vertex));
            vec4 oob_point_eq_radius = nearest_vertex_point - normalize(translation_point - local_starting_point) * x;
            if ((length_squared(oob_point_eq_radius - local_starting_point) <= length_squared(point_eq_radius- local_starting_point)) || !is_point_on_capped_line(point_eq_radius, vertex1, vertex2)) point_eq_radius = oob_point_eq_radius;
            vec4 collision_normal = normalize(point_eq_radius - closest_point_on_capped_line(point_eq_radius, vertex1, vertex2));
            point_eq_radius += collision_normal * 1e-5f;// sometimes calculations return negative miss... (probably floating point persition) better than walk trough objects
            //cout << "miss: " << length(point_eq_radius- closest_point_on_capped_line(point_eq_radius, vertex1, vertex2)) - radius << endl;
            if (std::isnan(point_eq_radius.x)) return {0, 0, 0, 0};
            return point_eq_radius;
            
        }
        return vec4(0);
    }


    InspectionResult closest_capsulecast_vs_line(float half_height, vec4 start, vec4 end, vec4 vertex1, vec4 vertex2){
        vec4 starting_up = start + vec4(0, half_height, 0, 0);
        vec4 starting_down = start - vec4(0, half_height, 0, 0);
        vec4 ending_up = end + vec4(0, half_height, 0, 0);
        vec4 ending_down = end - vec4(0, half_height, 0, 0);

        float dist;
        float new_dist;
        float dst_sq;

        vec4 a, b;
        vec4 na, nb;
        tie(a, b) = closest_points_between_line_segments_3d(starting_up, ending_up, vertex1, vertex2);
        float offset = half_height;
        dist = length_squared(a-b);
        start = starting_up;
        end = ending_up;
        dst_sq = length_squared(starting_up - a);

        if (half_height == 0){ return {start, end, a, offset, b, dst_sq};}

        tie(na, nb) = closest_points_between_line_segments_3d(starting_down, ending_down, vertex1, vertex2);
        new_dist = length_squared(na-nb);
        if (new_dist < dist) {
            dist = new_dist;
            a = na;
            b = nb;
            offset = -half_height;
            start = starting_down;
            end = ending_down;
            dst_sq = length_squared(starting_down - na);
        }
        tie(na, nb) = closest_points_between_line_segments_3d(starting_up, starting_down, vertex1, vertex2);
        new_dist = length_squared(na-nb);
        if (new_dist < dist) {
            dist = new_dist;
            a = na;
            b = nb;
            offset = clamp(nb.y-starting_up.y, -half_height * 2.0f, 0);
            start = starting_up + vec4(0, offset, 0, 0);
            end = ending_up + vec4(0, offset, 0, 0);
            offset += half_height;
            dst_sq = 0;
        }
        tie(na, nb) = closest_points_between_line_segments_3d(ending_up, ending_down, vertex1, vertex2);
        new_dist = length_squared(na-nb);
        if (new_dist < dist) {
            dist = new_dist;
            a = na;
            b = nb;
            offset = clamp(nb.y-ending_up.y, -half_height * 2.0f, 0);
            start = starting_up + vec4(0, offset, 0, 0);
            end = ending_up + vec4(0, offset, 0, 0);
            offset += half_height;
            dst_sq = length_squared(start - end);
        }


        //draw_line(starting_up, ending_up, {0, 1, 0});
        //draw_line(starting_down, ending_down, {0, 1, 0});
        //draw_line(starting_up, starting_down, {0, 1, 0});
        //draw_line(ending_up, ending_down, {0, 1, 0});
        
        return {
            start,
            end,
            a,
            offset,
            b,
            dst_sq
        };
    }

    void step(float delta, BDFAtlas* font_atlas){
        // copy
        vector<PhysicsPrimitive> dereferenced;
        for (auto obj: objects) {
            if (obj->type == TYPE_RIGID){
                //obj->velocity += gravity * delta;
                //obj->position += obj->velocity * delta;
            }
            dereferenced.push_back(*obj);
        }
        
        for (int a=0;a<objects.size();a++){
            // !ONLY CAPSULES CAN BE TYPE_RIGID
            PhysicsPrimitive first = PhysicsPrimitive{*objects[a]};
            if (first.type != TYPE_RIGID) continue; // iter only TYPE_RIGID BODIES (and update only self)
            vec4 starting_point = first.position;
            vector<vec4> parsed_normals;
            //first.velocity += gravity * delta;
            //first.position += first.velocity * delta;
            first.position += first.velocity;
            for (int b=0;b<objects.size();b++){ // intersection between rigids
                if (a==b) continue; // dont intersect self.
                PhysicsPrimitive* second = objects[b];
                //if (!intersects(first, second)) continue; // todo: check AABBs

                if (second->shape == SHAPE_CAPSULE){
                    if (!intersects(*objects[a], *second)){continue;};
                    vec4 first_center = objects[a]->position;
                    vec4 closest_point2 = closest_point(second, first_center);
                    vec4 closest_point1 = closest_point(&first, closest_point2);
                    vec4 result1 = apply_rounding(closest_point1, closest_point2, first.rounding);
                    vec4 result2 = apply_rounding(closest_point2, closest_point1, second->rounding);
                    vec4 vec = (result1 - result2);
                    if (vec == vec4(0)) continue;
                    if (dot(vec, first.position - second->position) > 0) continue; // prevent pushing inside
                    if (std::isnan(vec.x)) continue;
                    first.position -= vec * 0.5f;

                    //draw_arrow(first.position - vec, first.position, game->screen_pixel_size, {1, 0, 0});
                    //draw_arrow(first.position + first.velocity, first.position, game->screen_pixel_size, {0, 1, 0});

                    float system_energy = length(first.velocity) * first.mass + length(second->velocity) * second->mass;
                    float mid_bounciness = (first.bounciness + second->bounciness) * 0.5f;
                    float energy_amount = (second->mass)/(first.mass + second->mass)*system_energy;
                    vec4 vel = first.velocity;
                    vec4 collision_normal = normalize(vec);
                    vec2 projected = (dot(vel, collision_normal) / dot(collision_normal, collision_normal)) * vec2(collision_normal);
                    vec4 y_component = vec4(projected, 0, 0);
                    

                    if (dot(y_component, vel) > 0) y_component = -y_component;
                    vec4 plane_component = vel - vec4(projected, 0, 0);
                    if (vel == vec4{0., 0., 0., 0}){
                        plane_component = vec4(0);
                        y_component = -vec;
                    }
                    vec4 new_velocity = normalize(plane_component + y_component * mid_bounciness) * energy_amount;
                    if (dot(vec, new_velocity) >= 0) continue;
                    first.velocity = new_velocity;
                    //draw_arrow(objects[a]->position + new_velocity, objects[a]->position, game->screen_pixel_size, {0, 0, 1});
                    //cout << &first << " " << collision_normal.x << " " << collision_normal.y << endl;
                    dereferenced[a] = first; // write changes
                }               
            }
            //dereferenced[a] = first;

            // intersection between lines
            vec4 local_starting_point = starting_point;
            
            vec4 start = local_starting_point;
            vec4 end = first.position;
            float radius = first.rounding;
            float height = first.a.x;
            float half_height = height*0.5f;
            float radius_squared = (radius * radius);
            bool has_intersection = false;
            for (int b=0;b<objects.size();b++){ // check is intersection on the way
                if (a==b) continue;
                PhysicsPrimitive* second = objects[b];
                if (second->shape == SHAPE_LINE){
                    // todo: check aabbs?
                    if (dot(start-end, second->normal) < 0){continue;}
                    if (length_squared(start - closest_point_on_capped_line(start, second->position+second->a, second->position+second->b)) < radius_squared){continue;} // if start point intersects
                    InspectionResult res = closest_capsulecast_vs_line(half_height, start, end, second->position+second->a, second->position+second->b);
                    if (length_squared(res.point - res.line_point) < radius_squared){
                        has_intersection = true;
                        break;
                    }
                }
            }
            if (!has_intersection) {continue;} // no intersections on our way! no reason to use binsearch

            draw_line(start, end, {0, 1, 0});

            vec4 left = start;
            vec4 right = end;
            vec4 latest_uncollide = start;
            for (int i = 0; i < 256; i++){
                vec4 center = (left + right) / 2.0f;
                bool intersects = false;
                for (int b=0;b<objects.size();b++){
                    if (a==b) continue;
                    PhysicsPrimitive* second = objects[b];
                    if (second->shape == SHAPE_LINE){
                        if (dot(start-end, second->normal) < 0){continue;}
                        if (length_squared(start - closest_point_on_capped_line(start, second->position+second->a, second->position+second->b)) < radius_squared){continue;} // starting point MUST be outside line
                        InspectionResult res = closest_capsulecast_vs_line(half_height, left, center, second->position+second->a, second->position+second->b);
                        if (length_squared(res.point - res.line_point) < radius_squared){
                            intersects = true;
                            break;
                        }
                    }
                }
                if (intersects){
                    latest_uncollide = center;
                    if (length_squared(center - right) < 1e-7f) {break;}
                    right = center;
                } else {
                    left = center;
                }
            }
            draw_capsule(latest_uncollide, {first.rounding, first.a.x}, {1, 0, 1});

            //dereferenced[a] = first;
        }
        // sync
        for (int i=0;i<objects.size();i++){
            *objects[i] = dereferenced[i];
        }
        dereferenced.clear();
    }

    void gpu_step(float delta){
        glUseProgram(program);
        vector<PhysicsPrimitive> dereferenced;
        for (auto obj: objects) dereferenced.push_back(*obj);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, objects.size() * sizeof(PhysicsPrimitive), dereferenced.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, objects.size() * sizeof(PhysicsPrimitive), nullptr, GL_STATIC_DRAW);
        set_1i("scene_size", objects.size());
        set_1f("delta", delta);
        set_3f("gravity", gravity);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output_buffer);
        glDispatchCompute(objects.size(), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        vector<PhysicsPrimitive> new_objects(objects.size());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, objects.size() * sizeof(PhysicsPrimitive), &new_objects[0]);
        for (int i=0;i<objects.size();i++){
            //cout << new_objects[i].type << endl;
            *objects[i] = new_objects[i];
        }
        dereferenced.clear();
    }

    // physics logic
    private:
    vec4 gravity = vec4(0., -9.8, 0., 0.);
    vector<PhysicsPrimitive*> objects;

};


// todo: if collisions between two rigids will exists, i must order primitives by type (rigid steps first)