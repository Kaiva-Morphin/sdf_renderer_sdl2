#include "Game.h"
#include "textre_drawer.h"
#include "Skeleton.h"
#include "Functions.h"
#include "Animation.h"
#include "Characters.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;



int main(int argc, char ** argv)
{
    game = new Game();
    IMG_Init(IMG_INIT_PNG);
    game->init();
    

    int TEX_SIZE = 64;
    TextureDrawer drawer = TextureDrawer(TEX_SIZE, TEX_SIZE, TEX_SIZE);
    drawer.fill(DRAWER_WHITE);
    for (int z = 0; z < TEX_SIZE; z++)
    for (int y = 0; y < TEX_SIZE; y++)
    for (int x = 0; x < TEX_SIZE; x++)
    drawer.set_pixel(x, y, z, x > TEX_SIZE * 0.5 ? 0. : 255., y > TEX_SIZE * 0.5 ? 0. : 255., z > TEX_SIZE * 0.5 ? 0. : 255.);
    GLubyte* character_texture_data = drawer.get_data();

    SDF_Frag_Shader shader = SDF_Frag_Shader("assets/shaders/sdf_scene.frag");
    BoxBot boxbot;
    vec2 shader_texture_size = ivec2(48, 48);
    shader_texture_size = ivec2(128, 128);

    /*SDL_Surface* surface = IMG_Load("assets/images/ivan.png");

    shader.init(shader_texture_size.x, shader_texture_size.y, ivec3(64, 64, 1), (GLubyte*)surface->pixels);

    SDL_FreeSurface(surface);*/

    shader.init(shader_texture_size.x, shader_texture_size.y, ivec3(TEX_SIZE), character_texture_data);
    drawer.destroy();

    ObjectScene scene; 
    PrimitiveScene primitive_scene;


    

    Skeleton character;

    Primitive* torso = &primitive_scene.primitives[0];
    *torso = (BoxObject(vec3(0.3))).as_primitive();
    character.bones["torso"] = Bone{
        "torso",
        nullptr,
        {torso},
        with_offset(EYE4, {0, -0.5, 0}),
        vec4(0., 1., 0., 1.)
    };

    Primitive* torso_ = &primitive_scene.primitives[1];
    *torso_ = (BoxObject(vec3(0.4))).as_primitive();
    character.bones["torso_"] = Bone{
        "torso_",
        &character.bones["torso"],
        {torso_},
        with_offset(EYE4, {0, -0.5, 0}),
        vec4(0., 1., 0., 1.)
    };

    Primitive* head = &primitive_scene.primitives[2];
    //*head = (SphereObject( 0.5)).as_primitive();
    *head = (BoxObject(vec3(0.6, 0.5, 0.5))).as_primitive();
    character.bones["head"] = Bone{
        "head",
        &character.bones["torso_"],
        {head},
        with_offset(EYE4, {0, -0.5, 0}),
        vec4(0., 1., 0., 1.)
    };

    Primitive* larm = &primitive_scene.primitives[3];
    *larm = (SphereObject(0.5)).as_primitive();
    character.bones["larm"] = Bone{
        "larm",
        &character.bones["torso_"],
        {larm},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(-1., 1., 0., 1.)
    };

    Primitive* larm_ = &primitive_scene.primitives[4];
    *larm_ = (SphereObject(0.5)).as_primitive();
    character.bones["larm_"] = Bone{
        "larm_",
        &character.bones["larm"],
        {larm_},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(0., -1., 0., 1.)
    };

    Primitive* rarm = &primitive_scene.primitives[5];
    *rarm = (SphereObject(0.5)).as_primitive();
    character.bones["rarm"] = Bone{
        "rarm",
        &character.bones["torso_"],
        {rarm},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(1., 1., 0., 1.)
    };

    Primitive* rarm_ = &primitive_scene.primitives[6];
    *rarm_ = (SphereObject(0.5)).as_primitive();
    character.bones["rarm_"] = Bone{
        "rarm_",
        &character.bones["rarm"],
        {rarm_},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(0., -1., 0., 1.)
    };

    Primitive* lleg = &primitive_scene.primitives[7];
    *lleg = (SphereObject(0.5)).as_primitive();
    character.bones["lleg"] = Bone{
        "lleg",
        &character.bones["torso"],
        {lleg},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(-0.5, 0., 0., 1.)
    };

    Primitive* lleg_ = &primitive_scene.primitives[8];
    *lleg_ = (SphereObject( 0.5)).as_primitive();
    character.bones["lleg_"] = Bone{
        "lleg_",
        &character.bones["lleg"],
        {lleg_},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(0, -1, 0., 1.)
    };

    Primitive* rleg = &primitive_scene.primitives[9];
    *rleg = (SphereObject(0.5)).as_primitive();
    character.bones["rleg"] = Bone{
        "rleg",
        &character.bones["torso"],
        {rleg},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(0.5, 0., 0., 1.)
    };

    Primitive* rleg_ = &primitive_scene.primitives[10];
    *rleg_ = (SphereObject(0.5)).as_primitive();
    character.bones["rleg_"] = Bone{
        "rleg_",
        &character.bones["rleg"],
        {rleg_},
        with_offset(EYE4, {0, 0.5, 0}),
        vec4(0, -1, 0., 1.)
    };
    


    for (int i = 0; i < character.bones.size(); i++){
        primitive_scene.ordered_operations[i] =
            PrimitiveOperation{
                OPERATION_UNION,
                i-1,
                i,
                i,
                0.
            };
    }
    primitive_scene.operations = character.bones.size();
    primitive_scene.size = character.bones.size();

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 frag;
        uniform vec2 size;
        uniform vec2 target_size;
        
        mat4 bayerIndex = mat4(
            vec4(00.0/16.0, 12.0/16.0, 03.0/16.0, 15.0/16.0),
            vec4(08.0/16.0, 04.0/16.0, 11.0/16.0, 07.0/16.0),
            vec4(02.0/16.0, 14.0/16.0, 01.0/16.0, 13.0/16.0),
            vec4(10.0/16.0, 06.0/16.0, 09.0/16.0, 05.0/16.0));
        
        float dither8x8(float r) {    
            float threshold = bayerIndex[int(gl_FragCoord.x)%4][int(gl_FragCoord.y)%4];
            //return mix(r, r * floor(floor(r * 64.) / 64. + threshold) * 1.5, 1);
            return r > threshold ? 1:0;
        }

        void main() {
            vec3 color1 = vec3(0.7, 0.94, 0.99);
            color1 = vec3(0.102, 0.6, 0.713) * 0.7;
            vec3 color2 = vec3(0.04, 0.35, 0.45) * 0.7;
            vec2 uv = gl_FragCoord.xy / size.xy;
            float y = clamp((gl_FragCoord.y - size.y / 4.) / 32 - cos((gl_FragCoord.x-size.x/2.)/size.x * 4.) * 0.5, 0, 1);
            frag = vec4(mix(color1, color2, dither8x8(y)), 1.);

            //if (pixelPos.x < big.x && pixelPos.x > sml.x && pixelPos.y < big.y && pixelPos.y > sml.y) FragColor.b = 1.;

            //frag = pixelPos.y > target_size.y / 2 ? vec4(0.7, 0.94, 0.99, 1.) : vec4(0.04, 0.35, 0.45, 1.);
        }
    )";

    GLint success;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
    }


    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
    }
    
    BDFAtlas font_atlas = BDFAtlas("assets/fonts/orp/orp-italic.bdf", 1536);
    game->debugger.init(&font_atlas);
    
    glEnable(GL_BLEND);

    Animation sit("./assets/animations/bend_sit.json");
    game->start_timer();
    Animation anim(".\\assets\\animations\\tpose.json");
    cout << "parsing time: ";
    game->print_timer_end();
    float anim_tick = 0;
    bool playing = true;
    bool forced_loop = true;
    while (game->is_running())
    {
        float time = game->time();
        while (SDL_PollEvent(&event))
        {
            game->handle_event(event);
        }
        game->begin_main();
        
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform2f(glGetUniformLocation(shaderProgram, "target_size"), TARGET_WIDTH, TARGET_HEIGHT);
        glUniform2f(glGetUniformLocation(shaderProgram, "size"), game->get_screen_size().x, game->get_screen_size().y);
        game->draw_fullscreen_quad();
        glUseProgram(0);

        //game->end_main();

        //character.bones["torso_"].transform_bundle.transform = EulerXYZ(sin(time*3)*45, 0, 0);
        //character.bones["head"].transform_bundle.transform = EulerXYZ(sin(time*2)*45, 0, 0);
        //torso->transform = EulerXYZ(sin(time*2)*45, 0, 0);
        /*for (int i = 0; i < 10; i++){
            character.bones[to_string(i)].transform_bundle.transform = EulerZYX(0, 0, cos(time*2)*60);
        }*/
        //character.bones["torso"].transform_bundle.transform = EulerZYX(0, 0, cos(time)*60);
        //character.bones["torso_"].transform_bundle.transform = EulerZYX(0, 0, cos(time*2)*30);


        //character.bones["rarm"].transform_bundle.transform = EulerZYX(0, 0, -90+cos(time*2)*60);
        //character.bones["larm"].transform_bundle.transform = EulerZYX(0, 0, 90+cos(time*2)*60);
        

        //game->begin_main();

        std::ifstream file("file.txt");
        if (file.is_open()) {
            string str;
            getline(file, str);
            anim = Animation(str);
            file >> playing;
            game->debugger.register_line("iwannadie2", "playing: ", to_string(playing));
            file >> forced_loop;
            if (!forced_loop) forced_loop = anim.looped;
            if (!playing) file >> anim_tick;
            file.close();
        }
        if (playing){
            anim_tick += game->delta() * 20;
            if (forced_loop){
                if (anim_tick > anim.end_tick)
                    anim_tick = anim.return_tick; 
            } else {
                anim_tick = glm::clamp((float)anim_tick, (float)anim.start_tick, (float)anim.end_tick);
            }
        }

        //character.bones["torso"].transform_bundle.transform = EulerXYZ(sin(time*2)*45, 0, 0);
        game->debugger.register_line("iwannadie0", "anim_tick: ", to_string(anim_tick));
        game->start_timer();
        Pose pose = compute_pose(anim.get_pose(anim_tick));
        game->debugger.register_line("iwannadie1", "Pose get and compute: ", to_string(game->timer_end()));

        game->start_timer();
        character.apply_pose(pose); 
        game->debugger.register_line("iwannadie2", "Apply pose: ", to_string(game->timer_end()));

        shader.check_file_updates();
        shader.use();
        // x+
        shader.set_1f("time", time);
        shader.set_2f("texture_size", shader_texture_size.x, shader_texture_size.y);


        character.update_scene(&primitive_scene);
        shader.set_scene(&primitive_scene);

        //shader.set_position({cos(time) * 4, sin(time * 2) * 2, sin(time) * 4 + 4});

        shader.set_position({0, 0, 0});

        shader.update_map(0, vec2(0), vec3(0));
        shader.draw(game->screen_pixel_size);

        glUseProgram(0);
        

        game->end_main();

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game->draw_main();

        game->debugger.update_basic();
        game->debugger.draw(vec2(game->screen_rect.z, game->screen_rect.w));
        /*glBindTexture(GL_TEXTURE_2D, font_atlas.get_texture());
        vec2 src = font_atlas.get_glyph_size() * 8.0f / font_atlas.get_atlas_size();
        glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
        glTexCoord2f(src.x, src.y); glVertex2f(-1, -1);
        glTexCoord2f(0, src.y); glVertex2f(1, -1);
        glTexCoord2f(0, 0); glVertex2f(1, 1);
        glTexCoord2f(src.x, 0); glVertex2f(-1, 1);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);*/

        SDL_GL_SwapWindow(window);
        // todo: alpha checks for depth buffer draw :D
    }
    shader.destroy();
    glDeleteProgram(shaderProgram);
    IMG_Quit();
    game->destroy();
    return 0;
}