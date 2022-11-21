#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgOut;

const int MODE_LINEAR = 0;
const int MODE_RADIAL = 1;

uniform int mode = MODE_LINEAR;
uniform vec2 start;
uniform vec2 end;
uniform vec4 startColour;
uniform vec4 endColour;
uniform float falloff;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

    float gradientLength = dot(start - end, start - end);

    float dotDist;
    if (mode == MODE_LINEAR)
    {
        // Project the current pixel onto the gradient defined line to get it's offset
        dotDist = dot(start - end, pixel_coords - end) / gradientLength;
    }
    else if (mode == MODE_RADIAL)
    {
        dotDist = 1.0f - dot(pixel_coords - start, pixel_coords - start) / gradientLength;
    }
    else
    {
        dotDist = 0.0f;
    }
    float clampedDist = max(0.0f, min(dotDist, 1.0f));
    float falloffDist = pow(clampedDist, falloff);
    vec4 colour = mix(endColour, startColour, falloffDist);

    imageStore(imgOut, pixel_coords, colour);
}
