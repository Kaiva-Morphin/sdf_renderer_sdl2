#include "Physics.h"







int main(int argc, char ** argv)
{
    game = new Game();
    game->init();
    PhysicsSolver solver("");
    float dist;
    vec3 pa, pb;
    tie(dist, pa, pb) = solver.triangle_triangle({3, 3.5, 3}, {3, 4, 3}, {3, 4.5, 3}, {10, 0, 10}, {0, 0, 10}, {10, 0, 0});
    cout << dist << endl;
    cout << pa.x << ' ' << pa.y << ' ' << pa.z << endl;
    cout << pb.x << ' ' << pb.y << ' ' << pb.z << endl;

    game->destroy();
    return 0;
}

