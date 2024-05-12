#include <vector>
#include <string>
#include <unordered_map>
#include "sdf_primitives.h"






using namespace std;

















struct Bone{
    Bone* parent = nullptr;
    vector<Bone*> childs;
    vector<Primitive*> meshes;

    mat4 init_transform;
    vec4 offset;
};

class Character{
    public:
    unordered_map<string, Bone> bones;
};




