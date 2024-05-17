#include <vector>
#include <string>
#include <unordered_map>
#include "sdf_primitives.h"
#include "Pose.h"


using namespace std;


struct Bone{
    string name;
    Bone* parent = nullptr;
    vector<Bone*> childs;
    vector<Primitive*> meshes;

    mat4 init_transform; // *consts*
    vec4 offset; // *consts*

    BoneTransformBundle transform_bundle;
};

class Character{
    public:
    unordered_map<string, Bone> bones;



    void update_scene(PrimitiveScene* scene){
        unordered_map<string, BoneTransformBundle> cached_transforms;
        
        for (const auto& pair : bones){
            string bone_name = pair.first;
            BoneTransformBundle result_transform;
            Bone* bone = &bones[bone_name];
            Bone* parent = bone->parent;
            vector<Bone*> path;
            path.push_back(bone);
            while (parent != nullptr){
                path.push_back(parent);
                parent = parent->parent;
            }
            for (auto it = path.rbegin(); it != path.rend(); ++it) {
                Bone* b = *it;
                result_transform.position += (b->offset + b->transform_bundle.position) * result_transform.transform;// +  * b->transform_bundle.transform * result_transform.position;//result_transform.position;
                result_transform.transform = result_transform.transform * b->init_transform * b->transform_bundle.transform;
                result_transform.transform[3] = b->init_transform[3] + b->transform_bundle.transform[3];
            }
            for (Primitive* p : bones[bone_name].meshes){
                p->position = result_transform.position;
                p->transform = result_transform.transform;
            }
        }
    }

    void apply_pose(Pose pose){
        bones["torso"].transform_bundle = pose.torso;
        bones["torso_"].transform_bundle.transform = EulerZYX(pose.torso_bend, 0, 0);
        bones["head"].transform_bundle = pose.head;
        //bones["head_"].transform_bundle.transform = EulerZYX(0, pose.torso_bend, 0);
        bones["larm"].transform_bundle = pose.left_arm;
        bones["larm_"].transform_bundle.transform = EulerZYX(pose.left_arm_bend, 0, 0);
        bones["rarm"].transform_bundle = pose.right_arm;
        bones["rarm_"].transform_bundle.transform = EulerZYX(pose.right_arm_bend, 0, 0);
        bones["lleg"].transform_bundle = pose.left_leg;
        bones["lleg_"].transform_bundle.transform = EulerZYX(pose.left_leg_bend, 0, 0);
        bones["rleg"].transform_bundle = pose.right_leg;
        bones["rleg_"].transform_bundle.transform = EulerZYX(pose.right_leg_bend, 0, 0);
    }
};




