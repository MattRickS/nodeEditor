#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgOut;
// TODO: Add random seed. Either
// - Use 3D noise and seed is Z
// - Use as offset for x,y
// - Generate a different permutation table

uniform float frequency = 0.01;
uniform ivec2 offset = ivec2(0);

const int hashMask = 255;
const int gradientsMask2D = 7;
const float sqr2 = sqrt(2.0f);
// Random shuffle of 0-255
const int hashes2D[256] = int[256](
    151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
    140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
    247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
    57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
    74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
    60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
    65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
    200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
    52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
    207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
    119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
    129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
    218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
    81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
    184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
    222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180
);
const vec2 gradients2D[8] = vec2[8](
    vec2( 1.0f, 0.0f),
    vec2(-1.0f, 0.0f),
    vec2( 0.0f, 1.0f),
    vec2( 0.0f,-1.0f),
    normalize(vec2( 1.0f, 1.0f)),
    normalize(vec2(-1.0f, 1.0f)),
    normalize(vec2( 1.0f,-1.0f)),
    normalize(vec2(-1.0f,-1.0f))
);

float Smooth ( float t )
{
  // Fifth degree polynomial
  return t * t * t * ( t * ( t * 6.0f - 15.0f ) + 10.0f );
}

float perlin2D(vec2 point, float freq)
{
    point *= freq;
    int ix0 = int(floor( point.x ));
    int iy0 = int(floor( point.y ));
    float tx0 = point.x - ix0;
    float ty0 = point.y - iy0;
    float tx1 = tx0 - 1.0f;
    float ty1 = ty0 - 1.0f;
    ix0 &= hashMask;
    iy0 &= hashMask;
    int ix1 = ix0 + 1;
    int iy1 = iy0 + 1;

    int h0 = hashes2D[ix0];
    int h1 = hashes2D[ix1];

    vec2 g00 = gradients2D[hashes2D[h0 + iy0] & gradientsMask2D];
    vec2 g10 = gradients2D[hashes2D[h1 + iy0] & gradientsMask2D];
    vec2 g01 = gradients2D[hashes2D[h0 + iy1] & gradientsMask2D];
    vec2 g11 = gradients2D[hashes2D[h1 + iy1] & gradientsMask2D];

    float v00 = dot( g00, vec2( tx0, ty0 ) );
    float v10 = dot( g10, vec2( tx1, ty0 ) );
    float v01 = dot( g01, vec2( tx0, ty1 ) );
    float v11 = dot( g11, vec2( tx1, ty1 ) );

    float tx = Smooth( tx0 );
    float ty = Smooth( ty0 );
    return mix(
        mix( v00, v10, tx ),
        mix( v01, v11, tx ),
        ty ) * sqr2 * 0.5f + 0.5f;
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 pos = pixel_coords + offset;
    float noise = perlin2D(pos, frequency);
    imageStore(imgOut, pixel_coords, vec4(noise, noise, noise, 1));
}
