#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include "cmath"

#include "glm/glm.hpp"
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

#include <thread>

#define FIXED 1
#define MOVING 2
#define RIGID 4

#define CAPSULE 1
#define BOX 2
#define PYRAMID 4

struct alignas(16) PhysicsPrimitive{
    vec4 position = vec4(0.);
    vec4 velocity = vec4(0.);
    vec4 a = vec4(0.);
    vec4 b = vec4(0.);
    vec4 c = vec4(0.);
    float rounding = 0.;
    float bounciness = 1.;
    float mass = 1.;
    int type = FIXED;
    int shape = CAPSULE;
};

class PhysicsSolver{
    double remap(double value, double fromLow, double fromHigh, double toLow, double toHigh) {
        double normalized = (value - fromLow) / (fromHigh - fromLow);
        return toLow + normalized * (toHigh - toLow);
    }

    vec2 remap_vec2(vec2 value, vec2 fromLow, vec2 fromHigh, vec2 toLow, vec2 toHigh) {
        vec2 normalized = (value - fromLow) / (fromHigh - fromLow);
        return toLow + normalized * (toHigh - toLow);
    }

    // shader
    private:
    GLuint program = glCreateProgram();
    GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    GLuint input_buffer = 0;
    GLuint output_buffer = 0;
    string file_path;
    string current_src;
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

    void draw_line(vec2 pa, vec2 pb, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        vec2 ld = pa;
        vec2 ru = pb;
        float a = remap(ld.x, -half_sreen.x, half_sreen.x, -1, 1);
        float b = remap(ld.y, -half_sreen.y, half_sreen.y, -1, 1);
        float c = remap(ru.x, -half_sreen.x, half_sreen.x, -1, 1);
        float d = remap(ru.y, -half_sreen.y, half_sreen.y, -1, 1);


        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(a, b);
            glVertex2f(c, d);
            glColor3f(1., 1., 1.);
        glEnd();

    }

    void draw_box(vec2 pos, vec2 size, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        vec2 ld = pos - size * 0.5f;
        vec2 ru = pos + size * 0.5f;
        float a = remap(ld.x, -half_sreen.x, half_sreen.x, -1, 1);
        float b = remap(ld.y, -half_sreen.y, half_sreen.y, -1, 1);
        float c = remap(ru.x, -half_sreen.x, half_sreen.x, -1, 1);
        float d = remap(ru.y, -half_sreen.y, half_sreen.y, -1, 1);


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
    void draw_capsule(vec2 pos, vec2 size, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        vec2 ld = pos - size * vec2(1., 0.5);
        vec2 ru = pos + size * vec2(1., 0.5);

        vec2 uv_pos = remap_vec2(pos, -half_sreen, half_sreen, vec2(-1), vec2(1));
        vec2 radius = remap_vec2(vec2(size.x), -half_sreen, half_sreen, vec2(-1), vec2(1));
        float height = remap(size.y, -half_sreen.y, half_sreen.y, -1, 1);

        float lx = remap(ld.x, -half_sreen.x, half_sreen.x, -1, 1);
        float ly = remap(ld.y, -half_sreen.y, half_sreen.y, -1, 1);
        float rx = remap(ru.x, -half_sreen.x, half_sreen.x, -1, 1);
        float ry = remap(ru.y, -half_sreen.y, half_sreen.y, -1, 1);

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
    void draw_pyramid(){

    }
    vec3 get_color(int type){
        switch (type){
            case FIXED:
                return vec3(1., 0., 0.);
                break;
            case MOVING:
                return vec3(1., 0., 1.);
                break;
            case RIGID:
                return vec3(0., 0., 1.);
                break;
        }
        return vec3(0.);
    }
    void draw(vec2 screen_size){
        for (PhysicsPrimitive* object: objects){
            vec3 color = get_color(object->type);
            vec2 pos = vec2{object->position.x, object->position.y};
            
            switch (object->shape){
                case CAPSULE:
                    draw_capsule(pos, vec2(object->rounding, object->a.x), screen_size, color);
                    break;
                case BOX:
                    draw_box(pos, vec2(object->a.x, object->a.y), screen_size, color);
                    break;
                case PYRAMID:
                    break;
            }
        }
    }
    // constructors
    PhysicsPrimitive box(
        vec3 size=vec3(10.)
    ){
        return PhysicsPrimitive{
            vec4(0.),
            vec4(0.),
            vec4(size, 0.),
            vec4(0.),
            vec4(0.),
            0.,
            1.,
            1.,
            FIXED,
            BOX
        };
    }
    PhysicsPrimitive capsule(
        float height = 10.,
        float radius = 10
    ){
        return PhysicsPrimitive{
            vec4(0.),
            vec4(0.),
            vec4(height),
            vec4(0.),
            vec4(0.),
            radius,
            1.,
            1.,
            FIXED,
            CAPSULE
        };
    }
    PhysicsPrimitive pyramid(
        
    ){
        return PhysicsPrimitive{
            vec4(0.),
            vec4(0.),
            vec4(0.),
            vec4(0.),
            vec4(0.),
            0.,
            1.,
            1.,
            FIXED,
            PYRAMID
        };
    }

    void push(PhysicsPrimitive* p){
        objects.push_back(p);
    }


    vec4 get_aabb_max(PhysicsPrimitive p){
        switch (p.shape) {
            case BOX:
                return p.position + p.a * 0.5f;
                break;
            case CAPSULE:
                return p.position + vec4(p.rounding, p.rounding, p.rounding, 0.) + vec4(0., p.a.x * 0.5, 0., 0.);
                break;
            case PYRAMID:
                return vec4(0);
                break;
        }
        return vec4(0);
    }

    vec4 get_aabb_min(PhysicsPrimitive p){
        switch (p.shape) {
            case BOX:
                return p.position - p.a * 0.5f;
                break;
            case CAPSULE:
                return p.position - vec4(p.rounding, p.rounding, p.rounding, 0.) - vec4(0., p.a.x * 0.5, 0., 0.);
                break;
            case PYRAMID:
                return vec4(0);
                break;
        }
        return vec4(0);
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


    vec4 closest_point(PhysicsPrimitive p1, vec4 point){ // todo: make for 3d
        switch (p1.shape){
            case CAPSULE:{
                float y = point.y;
                if (p1.position.x == point.x) {
                    if (point.y < p1.position.y) return p1.position - vec4(0., p1.a.x*0.5f, 0., 0.);
                    else                         return p1.position + vec4(0., p1.a.x*0.5f, 0., 0.);
                } else {
                    y = glm::min(y, p1.position.y+p1.a.x*0.5f);
                    y = glm::max(y, p1.position.y-p1.a.x*0.5f);
                }
                return vec4(p1.position.x, y, p1.position.z, p1.position.w);
                break;
            }
            case BOX:{
                vec4 aabb_min = get_aabb_min(p1);
                vec4 aabb_max = get_aabb_max(p1);
                float x = point.x;
                float y = point.y;
                float z = point.z;
                if (is_point_in_AABB(p1, point)){ // todo: make for 3d
                    float dx1 = abs(x - aabb_min.x);
                    float dx2 = abs(x - aabb_max.x);
                    float dy1 = abs(y - aabb_min.y);
                    float dy2 = abs(y - aabb_max.y);
                    if (std::min(dx1, dx2) < std::min(dy1, dy2)){
                        if (dx1 > dx2){
                            x = aabb_max.x;
                        } else {
                            x = aabb_min.x;
                        }
                        return vec4(x, y, z, p1.position.w);
                    } else {
                        if (dy1 > dy2){
                            y = aabb_max.y;
                        } else {
                            y = aabb_min.y;
                        }
                        return vec4(x, y, z, p1.position.w);
                    }
                    return vec4(x, y, z, p1.position.w);
                }
                x = std::min(x, aabb_max.x);
                x = std::max(x, aabb_min.x);
                y = std::min(y, aabb_max.y);
                y = std::max(y, aabb_min.y);
                z = std::min(z, aabb_max.z);
                z = std::max(z, aabb_min.z);
                return vec4(x, y, z, p1.position.w);
                break;
            }
            //case PYRAMID:
            //    break;
        }
        return vec4(0);
    }

    void step(float delta, vec2 screen_size){
        // copy
        vector<PhysicsPrimitive> dereferenced;
        for (auto obj: objects) {
            if (obj->type == RIGID){
                //obj->velocity += gravity * delta;
                //obj->position += obj->velocity * delta;
            }
            dereferenced.push_back(*obj);
        }
        
        for (int a=0;a<objects.size();a++){
            // !ONLY CAPSULES CAN BE RIGID?
            PhysicsPrimitive first = PhysicsPrimitive{*objects[a]}; // clone?
            if (first.type != RIGID) continue; // iter only RIGID BODIES (update only self)
            for (int b=0;b<objects.size();b++){
                if (a==b) continue; // dont  iter self.
                PhysicsPrimitive second = PhysicsPrimitive{*objects[b]};
                if (!intersects(first, second)) continue; // check AABBs
                vec4 first_center = first.position;
                vec4 closest_point2 = closest_point(second, first_center);
                vec4 closest_point1 = closest_point(first, closest_point2);

                vec3 c = {};
                vec4 result1 = (closest_point1 == closest_point2) ? 
                    closest_point1 + normalize(closest_point1 - first.position) * first.rounding
                    :
                    (((is_point_in_AABB(second, closest_point1) != is_point_in_AABB(second, first.position))) && second.shape == BOX) ?
                        apply_inv_rounding(closest_point1, closest_point2, first.rounding)
                        :
                        apply_rounding(closest_point1, closest_point2, first.rounding);



                /*if (closest_point1 == closest_point2){
                    result1 = closest_point1 + normalize(closest_point1 - first.position) * first.rounding;
                } else {
                    if (((is_point_in_AABB(second, closest_point1) != is_point_in_AABB(second, first.position)))&& second.shape == BOX) { // todo c?t:f in shader
                        c = {1, 0, 0};
                        //result1 = apply_inv_rounding(closest_point1, closest_point2, first.rounding);
                        result1 = apply_inv_rounding(closest_point1, closest_point2, first.rounding);
                    } else result1 = apply_rounding(closest_point1, closest_point2, first.rounding);
                }*/

                /* ALMOST WORKING
                if (((is_point_in_AABB(second, closest_point1) != is_point_in_AABB(second, first.position)) || (closest_point1 == closest_point2))&& second.shape == BOX) { // todo c?t:f in shader
                    c = {1, 0, 0};
                    //result1 = apply_inv_rounding(closest_point1, closest_point2, first.rounding);
                    result1 = closest_point1 != closest_point2 ? apply_inv_rounding(closest_point1, closest_point2, first.rounding) : closest_point1 + normalize(closest_point1 - first.position) * first.rounding;
                } else result1 = apply_rounding(closest_point1, closest_point2, first.rounding);*/
                
                draw_capsule(closest_point1, {2, 0}, screen_size, c);
                draw_capsule(result1, {4, 0}, screen_size, {});
                //draw_capsule(closest_point2, {4, 0}, screen_size, {});



                dereferenced[a] = first; // write changes
            }
        }














        for (int a=0;a<objects.size();a++){
            PhysicsPrimitive obj1 = *objects[a];
            for (int b=0;b<objects.size();b++){
                continue;

                if (a == b) continue;
                PhysicsPrimitive obj2 = *objects[b];
                if (!(obj1.type == RIGID || obj2.type == RIGID)) continue;
                if (!intersects(obj1, obj2)) continue;
                vec4 center1 = obj1.position;
                vec4 center2 = obj2.position;
                vec4 closest_point1 = closest_point(obj1, center2);
                vec4 closest_point2 = closest_point(obj2, closest_point1);
                vec3 c1 = get_color(obj1.type);
                vec3 c2 = get_color(obj2.type);
                draw_capsule(closest_point1, {2, 0}, screen_size, c1);
                draw_capsule(closest_point2, {2, 0}, screen_size, c2);

                vec4 result1;
                if ((obj1.shape == CAPSULE) && (obj2.shape == BOX) && is_point_in_AABB(obj2, closest_point1)) result1 = apply_rounding(closest_point2 + (closest_point1 - closest_point2), closest_point1 + (closest_point1 - closest_point2), obj1.rounding);
                else result1 = apply_rounding(closest_point1, closest_point2, obj1.rounding);
                vec4 result2 = apply_rounding(closest_point2, closest_point1, obj2.rounding);

                draw_capsule(result1, {4, 0}, screen_size, c1);
                draw_capsule(result2, {4, 0}, screen_size, c2);

                vec4 vec = result1 - result2;

                if (dot(vec, center1 - center2) > 0) continue; // <- ->
                draw_line(result1, result2, screen_size, {0, 1, 1});
                if (obj1.type != RIGID){
                    obj2.position += vec;
                    //obj2.velocity = obj2.velocity - 2 * clamp(dot(obj2.velocity, vec), -1.0f, 1.0f) * vec;
                }
                else if (obj2.type != RIGID){
                    obj1.position -= vec;
                    //obj1.velocity = obj1.velocity - 2 * clamp(dot(obj1.velocity, vec), -1.0f, 1.0f) * vec;
                }
                else {
                    obj1.position -= vec * 0.5f;

                    //obj1.velocity = obj1.velocity - 2 * clamp(dot(obj1.velocity, -vec), -1.0f, 1.0f) * -vec;

                    obj2.position += vec * 0.5f;
                    
                    //reflect(vec2(0, 1), vec2(1, 0));
                    //obj2.velocity = obj2.velocity - 2 * clamp(dot(obj2.velocity, vec), -1.0f, 1.0f) * vec; // 
                }
                dereferenced[a] = obj1;
                dereferenced[b] = obj2;
            }
        }
        // sync
        for (int i=0;i<objects.size();i++){
            *objects[i] = dereferenced[i];
        }
        /*glUseProgram(program);
        vector<PhysicsPrimitive> dereferenced;
        for (auto obj: objects) dereferenced.push_back(*obj);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, objects.size() * sizeof(PhysicsPrimitive), dereferenced.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, objects.size() * sizeof(PhysicsPrimitive), nullptr, GL_STATIC_DRAW);
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
            cout << new_objects[i].type << endl;
            *objects[i] = new_objects[i];
        }

        dereferenced.clear();*/
    }

    // physics logic
    private:
    vec4 gravity = vec4(0., -9.8, 0., 0.);
    vector<PhysicsPrimitive*> objects;
    public:

};