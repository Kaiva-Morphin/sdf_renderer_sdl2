#include <string>
#include "BdfFont.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv){
    BDFFont bold_font = BDFAtlas("assets/fonts/orp/orp-bold.bdf").get_font();
    vector<int> sequence = {
        int(L'B'),
        int(L'e'),
        int(L'Ж'),
        int(L'@'),
        int(L'i'),
        int(L'I'),
    };
    for (int encoding : sequence){
        BDFChar bdf_char = bold_font.chars[encoding];
        cout << bdf_char.startx << ' ' << bdf_char.endx << endl;
        for (auto line : bdf_char.bitmap){
            for (int i = bdf_char.startx; i >= bdf_char.endx; --i) {
                cout << ((((line >> i) & 1) == 0 ? '.' : '@'));
            }
            cout << endl;
        }
    }
    return 0;
}