#include <iostream>
#include <fstream>
#include <filesystem>
#include <json.hpp>
#include <unordered_map>
#include <unordered_set>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace std;

#include "Animation.h"

int main(int argc, char** argv){
    Animation floss("./assets/animations/floss.json");


    cout<< floss.get_value(BONE_HEAD, PROPERTY_YAW, 4) << endl;

    /*fs::path directory_path = "./assets/animations";
    unordered_set<string> prepasses;
    unordered_set<string> emote_tags;
    unordered_set<string> easings;
    unordered_set<string> bone_names;
    unordered_set<string> names;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry)) {
            std::ifstream file(entry.path());
            if (!file.is_open()) {
                continue;
                return 1;
            }
            string file_content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
            file.close();
            try {
                json data = json::parse(file_content);
                for (auto item : data.items()) prepasses.insert(item.key());
                for (auto item : data["emote"].items()) emote_tags.insert(item.key());
                for (auto move : data["emote"]["moves"]){
                    for (auto item : move.items()){
                        if (item.key() == "easing") {
                            easings.insert(item.value());
                        }
                        if (item.key() == "turn" && item.value() != 0) {
                            cout << item.value() << endl;
                            cout << entry.path() << endl;
                        }
                        if (
                            item.key() != "tick" &&
                            item.key() != "easing" &&
                            item.key() != "turn"
                        ){
                            bone_names.insert(item.key());
                            for (auto item2 : move[item.key()].items()){
                                names.insert(item2.key());
                                if (
                                    item2.key() != "x" &&
                                    item2.key() != "y" &&
                                    item2.key() != "z" &&
                                    item2.key() != "yaw" &&
                                    item2.key() != "roll" &&
                                    item2.key() != "comment" &&
                                    item2.key() != "bend" &&
                                    item2.key() != "pitch"
                                ){
                                    cout << "KEY<" << item2.key() <<">" << endl;
                                    cout << "AT: " << entry.path() << endl;
                                }
                            }
                        }
                    }
                }

            } catch (json::parse_error& e) {
                //std::cerr << "Parsing failed: " << e.what() << std::endl;
                continue;
            }
        }
    }*/
    //cout << "Emote info" << endl;
    //for (auto name : prepasses) cout << name << endl;
    //cout << "Emote tags" << endl;
    //for (auto name : emote_tags) cout << name << endl;
    //cout << "Easings" << endl;
    //for (auto name : easings) cout << name << endl;
    //cout << "Bone names" << endl;
    //for (auto name : bone_names) cout << name << endl;
    //cout << "Bone items" << endl;
    //for (auto name : names) cout << name << endl;


    return 0;
}
/*


Emote info
    you can add // ignore
    name        // ignore
    emote       // ignore
    description // ignore
    author      // ignore

Emote tags
    returnTick
    moves      // ignore
    isLoop
    endTick
    degrees
    stopTick
    comment    // ignore
    beginTick

Easings
    linear
    InOutSine
    LINEAR
    EASEOUTQUAD
    CONSTANT
    EASEINQUAD
    EASEINOUTQUAD

Bone names
    leftItem  // ignore
    rightItem // ignore
    comment   // ignore
    torso
    rightLeg
    rightArm
    leftLeg
    head
    leftArm

Bone items
    x
    z
    y
    pitch
    roll
    yaw
    bend
    comment // ignore
    turn    // ignore
*/


/*
linear
InOutSine
LINEAR
EASEOUTQUAD
CONSTANT
EASEINQUAD
EASEINOUTQUAD
*/

/*
x
z
y
yaw
roll
pitch
bend
comment    // skip
*/

/*
leftItem  // skip
rightItem // skip
comment   // skip
torso
rightLeg
rightArm
leftLeg
head
leftArm++
*/