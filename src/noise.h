#include <cmath>
#include <cstdlib>

class PerlinNoise {
public:
    PerlinNoise();
    double noise(double x, double y, double z);

private:
    static const int permutationSize = 256;
    int permutation[2 * permutationSize];

    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);
};

PerlinNoise::PerlinNoise() {
    for (int i = 0; i < permutationSize; ++i) {
        permutation[i] = permutation[i + permutationSize] = rand() % permutationSize;
    }
}

double PerlinNoise::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise(double x, double y, double z) {
    int X = static_cast<int>(floor(x)) & 255;
    int Y = static_cast<int>(floor(y)) & 255;
    int Z = static_cast<int>(floor(z)) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int A = permutation[X] + Y;
    int AA = permutation[A] + Z;
    int AB = permutation[A + 1] + Z;
    int B = permutation[X + 1] + Y;
    int BA = permutation[B] + Z;
    int BB = permutation[B + 1] + Z;

    return lerp(w, lerp(v, lerp(u, grad(permutation[AA], x, y, z),
                                   grad(permutation[BA], x - 1, y, z)),
                           lerp(u, grad(permutation[AB], x, y - 1, z),
                                grad(permutation[BB], x - 1, y - 1, z))),
                   lerp(v, lerp(u, grad(permutation[AA + 1], x, y, z - 1),
                                grad(permutation[BA + 1], x - 1, y, z - 1)),
                           lerp(u, grad(permutation[AB + 1], x, y - 1, z - 1),
                                grad(permutation[BB + 1], x - 1, y - 1, z - 1))));
}


class SimplexNoise {
public:
    SimplexNoise();
    double noise(double xin, double yin, double zin);

private:
    int grad3[12][3] = {
        {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
        {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
        {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
    };

    int p[512];
    int perm[512];

    int fastfloor(double x);
    double dot(int g[], double x, double y, double z);
    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);
};

SimplexNoise::SimplexNoise() {
    for (int i = 0; i < 512; ++i) {
        p[i] = grad3[i%12][0] + grad3[i%12][1] + grad3[i%12][2];
        perm[i] = i;
    }

    for (int i = 0; i < 512; ++i) {
        int r = i + (rand() % (512 - i));
        int temp = perm[i];
        perm[i] = perm[r];
        perm[r] = temp;
    }
}

int SimplexNoise::fastfloor(double x) {
    return x > 0 ? static_cast<int>(x) : static_cast<int>(x) - 1;
}

double SimplexNoise::dot(int g[], double x, double y, double z) {
    return g[0]*x + g[1]*y + g[2]*z;
}

double SimplexNoise::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double SimplexNoise::lerp(double t, double a, double b) {
    return a + t * (b - a);
}

double SimplexNoise::grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

double SimplexNoise::noise(double xin, double yin, double zin) {
    // Skewing and unskewing factors for 3D
    static const double F3 = 1.0/3.0;
    static const double G3 = 1.0/6.0;

    // Noise contributions from each corner
    double n0, n1, n2, n3;

    // Skew the input space to determine which simplex cell we're in
    double s = (xin+yin+zin)*F3;
    int i = fastfloor(xin + s);
    int j = fastfloor(yin + s);
    int k = fastfloor(zin + s);

    // Unskew the cell origin back to (x,y,z) space
    double t = (i+j+k)*G3;
    double X0 = i - t;
    double Y0 = j - t;
    double Z0 = k - t;
    double x0 = xin - X0;
    double y0 = yin - Y0;
    double z0 = zin - Z0;

    // Determine which simplex we are in
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        } else if (x0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 0; k2 = 1;
        } else {
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 1; j2 = 0; k2 = 1;
        }
    } else {
        if (y0 < z0) {
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 0; j2 = 1; k2 = 1;
        } else if (x0 < z0) {
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 0; j2 = 1; k2 = 1;
        } else {
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        }
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z),
    // and a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z),
    // where c = 1/6.
    double x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;
    double x2 = x0 - i2 + 2.0*G3; // Offsets for third corner in (x,y,z) coords
    double y2 = y0 - j2 + 2.0*G3;
    double z2 = z0 - k2 + 2.0*G3;
    double x3 = x0 - 1.0 + 3.0*G3; // Offsets for last corner in (x,y,z) coords
    double y3 = y0 - 1.0 + 3.0*G3;
    double z3 = z0 - 1.0 + 3.0*G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = p[ii + p[jj + p[kk]]] % 12;
    int gi1 = p[ii + i1 + p[jj + j1 + p[kk + k1]]] % 12;
    int gi2 = p[ii + i2 + p[jj + j2 + p[kk + k2]]] % 12;
    int gi3 = p[ii + 1 + p[jj + 1 + p[kk + 1]]] % 12;

    // Calculate the contribution from each corner
    double t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }

    double t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }

    double t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }

    double t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 32.0 * (n0 + n1 + n2 + n3);
}