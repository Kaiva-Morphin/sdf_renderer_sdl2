#include <string>
#include "Game.h"
#include "Skeleton.h"

using namespace std;

class Character{
    protected:
    Skeleton skeleton;
    PrimitiveScene mesh_scene;
    public:
    SDF_Frag_Shader shader;
    void apply_pose(Pose pose){
        skeleton.apply_pose(pose);
        skeleton.update_scene();
        shader.set_scene(&mesh_scene);
    }
    Character(){}
    ~Character(){
        shader.destroy();
    }
    virtual void time_step(float delta_time){}
};


class BoxBot : public Character{
    public:
    BoxBot() : Character(){
        Primitive* torso = &mesh_scene.primitives[0];
        *torso = (BoxObject(vec3(0.3))).as_primitive();
        skeleton.bones["torso"] = Bone{
            "torso",
            nullptr,
            {torso},
            with_offset(EYE4, {0, -0.5, 0}),
            vec4(0., 1., 0., 1.)
        };

        Primitive* torso_ = &mesh_scene.primitives[1];
        *torso_ = (BoxObject(vec3(0.4))).as_primitive();
        skeleton.bones["torso_"] = Bone{
            "torso_",
            &skeleton.bones["torso"],
            {torso_},
            with_offset(EYE4, {0, -0.5, 0}),
            vec4(0., 1., 0., 1.)
        };

        Primitive* head = &mesh_scene.primitives[2];
        //*head = (SphereObject( 0.5)).as_primitive();
        *head = (BoxObject(vec3(0.6, 0.5, 0.5))).as_primitive();
        skeleton.bones["head"] = Bone{
            "head",
            &skeleton.bones["torso_"],
            {head},
            with_offset(EYE4, {0, -0.5, 0}),
            vec4(0., 1., 0., 1.)
        };

        Primitive* larm = &mesh_scene.primitives[3];
        *larm = (SphereObject(0.5)).as_primitive();
        skeleton.bones["larm"] = Bone{
            "larm",
            &skeleton.bones["torso_"],
            {larm},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(-1., 1., 0., 1.)
        };

        Primitive* larm_ = &mesh_scene.primitives[4];
        *larm_ = (SphereObject(0.5)).as_primitive();
        skeleton.bones["larm_"] = Bone{
            "larm_",
            &skeleton.bones["larm"],
            {larm_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0., -1., 0., 1.)
        };

        Primitive* rarm = &mesh_scene.primitives[5];
        *rarm = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rarm"] = Bone{
            "rarm",
            &skeleton.bones["torso_"],
            {rarm},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(1., 1., 0., 1.)
        };

        Primitive* rarm_ = &mesh_scene.primitives[6];
        *rarm_ = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rarm_"] = Bone{
            "rarm_",
            &skeleton.bones["rarm"],
            {rarm_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0., -1., 0., 1.)
        };

        Primitive* lleg = &mesh_scene.primitives[7];
        *lleg = (SphereObject(0.5)).as_primitive();
        skeleton.bones["lleg"] = Bone{
            "lleg",
            &skeleton.bones["torso"],
            {lleg},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(-0.5, 0., 0., 1.)
        };

        Primitive* lleg_ = &mesh_scene.primitives[8];
        *lleg_ = (SphereObject( 0.5)).as_primitive();
        skeleton.bones["lleg_"] = Bone{
            "lleg_",
            &skeleton.bones["lleg"],
            {lleg_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0, -1, 0., 1.)
        };

        Primitive* rleg = &mesh_scene.primitives[9];
        *rleg = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rleg"] = Bone{
            "rleg",
            &skeleton.bones["torso"],
            {rleg},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0.5, 0., 0., 1.)
        };

        Primitive* rleg_ = &mesh_scene.primitives[10];
        *rleg_ = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rleg_"] = Bone{
            "rleg_",
            &skeleton.bones["rleg"],
            {rleg_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0, -1, 0., 1.)
        };
        


        for (int i = 0; i < skeleton.bones.size(); i++){
            mesh_scene.ordered_operations[i] =
                PrimitiveOperation{
                    OPERATION_UNION,
                    i-1,
                    i,
                    i,
                    0.
                };
        }

        mesh_scene.operations = skeleton.bones.size();
        mesh_scene.size = skeleton.bones.size();
    }
};

class PolyMorph : public Character{
    float local_time = 0;
    float wrapped_time = float(int(local_time * 200) % (12 * 100)) / 100;
    float a = glm::clamp(wrapped_time,      0.0f, 1.0f);
    float b = glm::clamp(wrapped_time - 2,  0.0f, 1.0f);
    float c = glm::clamp(wrapped_time - 4,  0.0f, 1.0f);
    float d = glm::clamp(wrapped_time - 6,  0.0f, 1.0f);
    float e = glm::clamp(wrapped_time - 8,  0.0f, 1.0f);
    float f = glm::clamp(wrapped_time - 10, 0.0f, 1.0f);
    public:
    PolyMorph() : Character(){
        skeleton.bones["torso"] = Bone{
            "torso",
            nullptr,
            {},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };
        Bone* root = &skeleton.bones["torso"];

        Primitive* shere = &mesh_scene.primitives[0];
        *shere = (SphereObject(1)).as_primitive();
        skeleton.bones["sphere"] = Bone{
            "sphere",
            root,
            {shere},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* line = &mesh_scene.primitives[1];
        *line = (LineObject(vec3(-1), vec3(1), 1)).as_primitive();
        skeleton.bones["line"] = Bone{
            "line",
            root,
            {line},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* cyl = &mesh_scene.primitives[2];
        *cyl = (CylinderObject(vec3(-1), vec3(1), 1)).as_primitive();
        skeleton.bones["cyl"] = Bone{
            "cyl",
            root,
            {cyl},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* box = &mesh_scene.primitives[3];
        *box = (BoxObject(vec3(1))).as_primitive();
        skeleton.bones["box"] = Bone{
            "box",
            root,
            {box},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* orb1 = &mesh_scene.primitives[4];
        *orb1 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["orb1"] = Bone{
            "orb1",
            root,
            {orb1},
            with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400, local_time * 40),
            vec4(0., 0., 0., 1.)
        };

        Primitive* orb2 = &mesh_scene.primitives[5];
        *orb2 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["orb2"] = Bone{
            "orb2",
            root,
            {orb2},
            with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 120, local_time * 40),
            vec4(0., 0., 0., 1.)
        };
        Primitive* orb3 = &mesh_scene.primitives[6];
        *orb3 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["orb3"] = Bone{
            "orb3",
            root,
            {orb3},
            with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 240, local_time * 40),
            vec4(0., 0., 0., 1.)
        };

        Primitive* shere1 = &mesh_scene.primitives[7];
        *shere1 = (SphereObject(1)).as_primitive();
        skeleton.bones["shere1"] = Bone{
            "shere1",
            root,
            {shere1},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* shere2 = &mesh_scene.primitives[8];
        *shere2 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["shere2"] = Bone{
            "shere2",
            root,
            {shere2},
            with_offset(EYE4, {0.5, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        mesh_scene.operations = 9;
        mesh_scene.ordered_operations[0] = PrimitiveOperation{3, 0, 1, 1, a};
        mesh_scene.ordered_operations[1] = PrimitiveOperation{3, 1, 2, 2, b};
        mesh_scene.ordered_operations[2] = PrimitiveOperation{3, 2, 3, 3, c};
        mesh_scene.ordered_operations[3] = PrimitiveOperation{0, 4, 5, 4, 0};
        mesh_scene.ordered_operations[4] = PrimitiveOperation{0, 4, 6, 4, 0};
        mesh_scene.ordered_operations[5] = PrimitiveOperation{3, 3, 4, 6, d};
        mesh_scene.ordered_operations[6] = PrimitiveOperation{1, 8, 7, 7, 0.5};
        mesh_scene.ordered_operations[7] = PrimitiveOperation{3, 6, 7, 8, 0};
        mesh_scene.ordered_operations[8] = PrimitiveOperation{3, 8, 0, 8, f};

        // six stages:
        // SPHERE -> LINE -> CYL -> BOX -> ORBS -> DEATH_STAR
        mesh_scene.size = skeleton.bones.size();
    }
    void time_step(float delta_time){
        local_time += delta_time * 30;
        wrapped_time = float(int(local_time * 200) % (12 * 100)) / 100;
        a = glm::clamp(wrapped_time,      0.0f, 1.0f);
        b = glm::clamp(wrapped_time - 2,  0.0f, 1.0f);
        c = glm::clamp(wrapped_time - 4,  0.0f, 1.0f);
        d = glm::clamp(wrapped_time - 6,  0.0f, 1.0f);
        e = glm::clamp(wrapped_time - 8,  0.0f, 1.0f);
        f = glm::clamp(wrapped_time - 10, 0.0f, 1.0f);
        mesh_scene.ordered_operations[0] = PrimitiveOperation{3, 0, 1, 1, a};
        mesh_scene.ordered_operations[1] = PrimitiveOperation{3, 1, 2, 2, b};
        mesh_scene.ordered_operations[2] = PrimitiveOperation{3, 2, 3, 3, c};
        mesh_scene.ordered_operations[5] = PrimitiveOperation{3, 3, 4, 6, d};
        mesh_scene.ordered_operations[8] = PrimitiveOperation{3, 8, 0, 8, f};
        skeleton.bones["orb1"].init_transform = with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 0, local_time * 40);
        skeleton.bones["orb2"].init_transform = with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 120, local_time * 40);
        skeleton.bones["orb3"].init_transform = with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 240, local_time * 40);
    }

};

class BloodKnight : public Character{
    public:
    BloodKnight() : Character(){
        skeleton.bones["torso"] = Bone{
            "torso",
            nullptr,
            {},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };
        Bone* root = &skeleton.bones["torso"];

        Primitive* shere = &mesh_scene.primitives[0];
        *shere = (SphereObject(1)).as_primitive();
        skeleton.bones["sphere"] = Bone{
            "sphere",
            root,
            {shere},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };
        mesh_scene.operations = 1;
        mesh_scene.size = 1;
        mesh_scene.ordered_operations[0] = PrimitiveOperation{0, 0, 0, 0, 0};
        
    }
};

class LivingConstruct : public Character{
    public:
    LivingConstruct() : Character(){
        
    }
};

class WoodEn : public Character{
    public:
    WoodEn() : Character(){
        
    }
};

