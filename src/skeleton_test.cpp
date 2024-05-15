#include "Game.h"
#include "textre_drawer.h"
#include "Skeleton.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;

mat4 with_offset(mat4 mat, vec3 offset){
    mat[3] = vec4(offset, mat[3][3]);
    return mat;
}


mat4x4 EulerXYZ(float anglex, float angley, float anglez){
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

mat4x4 EulerZYX(float anglex, float angley, float anglez){
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

    SDF_Frag_Shader shader = SDF_Frag_Shader("assets/shaders/sdf_scene.frag", &game->debugger);
    vec2 shader_texture_size = ivec2(48, 48);
    shader_texture_size = ivec2(128, 128);

    /*SDL_Surface* surface = IMG_Load("assets/images/ivan.png");

    shader.init(shader_texture_size.x, shader_texture_size.y, ivec3(64, 64, 1), (GLubyte*)surface->pixels);

    SDL_FreeSurface(surface);*/

    shader.init(shader_texture_size.x, shader_texture_size.y, ivec3(TEX_SIZE), character_texture_data);
    drawer.destroy();

    ObjectScene scene; 
    PrimitiveScene primitive_scene;

    int size = 6;

    for (int i = 0; i < size; i++){
        primitive_scene.ordered_operations[i] =
            PrimitiveOperation{
                OPERATION_UNION,
                i-1,
                i,
                i,
                0.
            };
    }
    primitive_scene.operations = size;
    primitive_scene.size = size;
    

    Character character;

    Primitive* root = &primitive_scene.primitives[0];
    *root = (SphereObject(vec3(0., 0., 0.), 1)).as_primitive();
    character.bones["0"] = Bone{
        "0",
        nullptr,
        {},
        {root},
        with_offset(EYE4, {0, -1, 0}),
        vec4(0., 0., 0., 1.)
    };

    for (int i = 1; i < size; i++){
        Primitive* p = &primitive_scene.primitives[i];
        *p = (SphereObject(vec3(0., 0., 0.), 1)).as_primitive();
        character.bones[to_string(i)] = Bone{
            to_string(i),
            &character.bones[to_string(i-1)],
            {},
            {p},
            with_offset(EYE4, {0, -1, 0}),
            vec4(0., 2., 0., 1.)
        };
    }
    
    

    /*Primitive* chest = &primitive_scene.primitives[0];
    *chest = (BoxObject(vec3(0., 0., 0.), vec3(0.8, 0.6, 0.4))).as_primitive();
    character.bones["chest"] = Bone{
        "chest",
        nullptr,
        {},
        {chest},
        EYE4,
        vec4(0., 0., 0., 1.)
    };
    character.bones["chest"].init_transform[3] = vec4(0, -0.6, 0, 1);

    Primitive* chest_upper = &primitive_scene.primitives[1];
    *chest_upper = (BoxObject(vec3(0., 0., 0.), vec3(0.8, 0.6, 0.4))).as_primitive();
    character.bones["chest_upper"] = Bone{
        "chest_upper",
        &character.bones["chest"],
        {},
        {chest_upper},
        EYE4,
        vec4(0., 0.8, 0., 1.)
    };
    character.bones["chest_upper"].init_transform[3] = vec4(0, -0.4, 0, 1);

    Primitive* head = &primitive_scene.primitives[2];
    *head = (BoxObject(vec3(0., 0, 0.), vec3(0.8, 0.8, 0.8))).as_primitive();
    character.bones["head"] = Bone{
        "head",
        &character.bones["chest_upper"],
        {},
        {head},
        EYE4,
        vec4(0., 1.8, 0., 1.),
    };
    character.bones["head"].init_transform[3] = vec4(0, -0.8, 0, 1);

    Primitive* left_arm = &primitive_scene.primitives[3];
    *left_arm = (BoxObject(vec3(0., 0., 0.), vec3(0.4, 0.8, 0.4))).as_primitive();
    character.bones["left_arm"] = Bone{
        "left_arm",
        &character.bones["chest_upper"],
        {},
        {left_arm},
        EulerXYZ(0, 0, 0),
        vec4(1.2, 1.4, 0., 1.),
    };*/

    /*BoxObject* chest_upper = new BoxObject(vec3(0., 0., 0.), vec3(0.8, 0.8, 0.4));
    chest_upper->position.y = 0;
    scene.objects.push_back(chest_upper);

    character.bones["chest"] = Bone{

    };*/

    /*BoxObject* head = new BoxObject(vec3(0., 0., 0.), vec3(5., 5., 0.2));
    head->position.y = 0;
    scene.objects.push_back(head);*/

    /*
    scene.primitives[0].a.xyz = vec3(0.8);
    scene.primitives[0].position.xyz = vec3(0, 1.6+0.8, 0);

    scene.primitives[1].a.xyz = vec3(0.8, 1.6, 0.4);
    scene.primitives[1].position.xyz = vec3(0, 0, 0);

    scene.primitives[2].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[2].position.xyz = vec3(1.4, 0.8, 0);
    
    scene.primitives[3].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[3].position.xyz = vec3(1.4, -0.8, 0);

    scene.primitives[4].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[4].position.xyz = vec3(-1.4, 0.8, 0);

    scene.primitives[5].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[5].position.xyz = vec3(-1.4, -0.8, 0);

    scene.primitives[6].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[6].position.xyz = vec3(-0.4, -2.4, 0);

    scene.primitives[7].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[7].position.xyz = vec3(-0.4, -4, 0);

    scene.primitives[8].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[8].position.xyz = vec3(0.4, -2.4, 0);

    scene.primitives[9].a.xyz = vec3(0.4, 0.8, 0.4);
    scene.primitives[9].position.xyz = vec3(0.4, -4, 0);
    */

    //scene.update_primitive_scene(&primitive_scene);

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

        //character.bones["chest"].transform_bundle.transform = EulerXYZ(sin(time*2)*45, 0, 0);
        //character.bones["chest_upper"].transform_bundle.transform = EulerXYZ(sin(time*3)*45, 0, 0);
        //character.bones["head"].transform_bundle.transform = EulerXYZ(sin(time*2)*45, 0, 0);
        //chest->transform = EulerXYZ(sin(time*2)*45, 0, 0);
        for (int i = 0; i < 10; i++){
            character.bones[to_string(i)].transform_bundle.transform = EulerZYX(0, 0, cos(time*2)*60);
        }


        //game->begin_main();
        
        
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
        //game->debugger.update_basic();
        //game->debugger.draw(game->screen_pixel_size);

        game->end_main();

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game->draw_main();

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