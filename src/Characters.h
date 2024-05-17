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
    Character(){}
    ~Character(){
        shader.destroy();
    }
};


class BoxBot : public Character{
    public:
    void apply_pose(Pose pose){
        skeleton.apply_pose(pose);
        skeleton.update_scene(&mesh_scene);
        shader.set_scene(&mesh_scene);
    }
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
    public:
    PolyMorph() : Character(){
        
    }
};

class BloodKnight : public Character{
    public:
    BloodKnight() : Character(){
        
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

