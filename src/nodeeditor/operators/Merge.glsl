#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn0;
layout(rgba32f, binding=1) uniform image2D imgIn1;
layout(rgba32f, binding=2) uniform image2D imgIn2;
layout(rgba32f, binding=3) uniform image2D imgOut;

const int MODE_ATOP = 0;
const int MODE_AVERAGE = 1;
const int MODE_COLORBURN = 2;
const int MODE_COLORDODGE = 3;
const int MODE_CONJOINTOVER = 4;
const int MODE_COPY = 5;
const int MODE_DIFFERENCE = 6;
const int MODE_DISJOINTOVER = 7;
const int MODE_DIVIDE = 8;
const int MODE_EXCLUSION = 9;
const int MODE_FROM = 10;
const int MODE_GEOMETRIC = 11;
const int MODE_HARDLIGHT = 12;
const int MODE_HYPOT = 13;
const int MODE_IN = 14;
const int MODE_MASK = 15;
const int MODE_MATTE = 16;
const int MODE_MAX = 17;
const int MODE_MIN = 18;
const int MODE_MINUS = 19;
const int MODE_MULTIPLY = 20;
const int MODE_OUT = 21;
const int MODE_OVER = 22;
const int MODE_OVERLAY = 23;
const int MODE_PLUS = 24;
const int MODE_SCREEN = 25;
const int MODE_SOFTLIGHT = 26;
const int MODE_STENCIL = 27;
const int MODE_UNDER = 28;
const int MODE_XOR = 29;

uniform int mode = MODE_OVER;
uniform float blend = 1.0f;
uniform bool alphaMask = true;
uniform int maskChannel = 3;
uniform bool _ignoreImage2 = false;

float multiply(float A, float B)
{
    return (A < 0 && B < 0) ? B : A * B;
}
float screen(float A, float B)
{
    if (0 <= A && A <= 1 && 0 <= B && B <= 1) { return B + A - A * B; }
    else if (B > A) { return B; }
    else { return A; }
}

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    float mask = 1;
    if (!_ignoreImage2)
    {
        mask = imageLoad(imgIn2, pixel)[maskChannel];
    }

    vec4 A = imageLoad(imgIn0, pixel);
    vec4 B = imageLoad(imgIn1, pixel);

    vec4 value;
    switch (mode)
    {
    case MODE_ATOP:
        value = B * A.a + A * (1 - B.a);
        break;
    case MODE_AVERAGE:
        value = (A + B) * 0.5f;
        break;
    // case MODE_COLORBURN:
    //     break;
    // case MODE_COLORDODGE:
    //     break;
    case MODE_CONJOINTOVER:
        value = (B.a > A.a) ? B : B + A * (1 - B.a) / A.a;
        break;
    case MODE_COPY:
        value = A;  // only useful if `blend` is also used
        break;
    case MODE_DIFFERENCE:
        value = abs(B - A);
        break;
    case MODE_DISJOINTOVER:
        value = ((B.a + A.a) < 1) ? A + B : B + A * (1 - B.a) / A.a;
        break;
    case MODE_DIVIDE:
        // Prevents two negatives becoming a positive
        value = vec4(
            (A.r < 0 && B.r < 0) ? 0 : B.r / A.r,
            (A.g < 0 && B.g < 0) ? 0 : B.g / A.g,
            (A.b < 0 && B.b < 0) ? 0 : B.b / A.b,
            (A.a < 0 && B.a < 0) ? 0 : B.a / A.a
        );
        break;
    case MODE_EXCLUSION:
        value = A + B - 2 * A * B;
        break;
    case MODE_FROM:
        value = A - B;
        break;
    case MODE_GEOMETRIC:
        value = (2 * A * B) / (A + B);
        break;
    case MODE_HARDLIGHT:
        value = vec4(
            (B.r < 0.5f) ? multiply(A.r, B.r) : screen(A.r, B.r),
            (B.g < 0.5f) ? multiply(A.g, B.g) : screen(A.g, B.g),
            (B.b < 0.5f) ? multiply(A.b, B.b) : screen(A.b, B.b),
            (B.a < 0.5f) ? multiply(A.a, B.a) : screen(A.a, B.a)
        );
        break;
    case MODE_HYPOT:
        // TODO: Diagonal square... just 1.414213562 multiplier ?
        value = sqrt(A * A + B * B);
        break;
    case MODE_IN:
        value = B * A.a;
        break;
    case MODE_MASK:
        value = A * B.a;
        break;
    case MODE_MATTE:
        value = B * B.a + A.a * (1 - B.a);
        break;
    case MODE_MAX:
        value = max(B, A);
        break;
    case MODE_MIN:
        value = min(B, A);
        break;
    case MODE_MINUS:
        value = A - B;
        break;
    case MODE_MULTIPLY:
        value = vec4(
            multiply(A.r, B.r),
            multiply(A.g, B.g),
            multiply(A.b, B.b),
            multiply(A.a, B.a)
        );
        break;
    case MODE_OUT:
        value = B * (1 - A.a);
        break;
    case MODE_OVER:
        value = B + A * (1 - B.a);
        break;
    case MODE_OVERLAY:
        value = vec4(
            (A.r < 0.5f) ? multiply(A.r, B.r) : screen(A.r, B.r),
            (A.g < 0.5f) ? multiply(A.g, B.g) : screen(A.g, B.g),
            (A.b < 0.5f) ? multiply(A.b, B.b) : screen(A.b, B.b),
            (A.a < 0.5f) ? multiply(A.a, B.a) : screen(A.a, B.a)
        );
        break;
    case MODE_PLUS:
        value = A + B;
        break;
    case MODE_SCREEN:
        value = vec4(
            screen(A.r, B.r),
            screen(A.g, B.g),
            screen(A.b, B.b),
            screen(A.a, B.a)
        );
        break;
    case MODE_SOFTLIGHT:
        value = vec4(
            ((A.r * B.r) < 1) ? A.r * (2 * B.r + A.r * (1 - A.r * B.r)) : 2 * A.r * B.r,
            ((A.g * B.g) < 1) ? A.g * (2 * B.g + A.g * (1 - A.g * B.g)) : 2 * A.g * B.g,
            ((A.b * B.b) < 1) ? A.b * (2 * B.b + A.b * (1 - A.b * B.b)) : 2 * A.b * B.b,
            ((A.a * B.a) < 1) ? A.a * (2 * B.a + A.a * (1 - A.a * B.a)) : 2 * A.a * B.a
        );
        break;
    case MODE_STENCIL:
        value = A * (1 - B.a);
        break;
    case MODE_UNDER:
        value = B * (1 - A.a) + A;
        break;
    case MODE_XOR:
        value = A * (1 - B.a) + B * (1 - A.a);
        break;
    }

    value = mix(A, value, blend * mask);
    if (alphaMask)
    {
        value.a = B.a + A.a - B.a * A.a;
    }

    imageStore(imgOut, pixel, value);
}
