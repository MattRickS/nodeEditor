#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform vec2 start;
uniform vec2 end;
uniform vec4 startColour;
uniform vec4 endColour;
uniform float falloff;
uniform bool _ignoreImage0 = false;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

    float dotDist = dot(start - end, pixel_coords - end) / dot(start - end, start - end);
    float clampedDist = max(0.0f, min(dotDist, 1.0f));
    float falloffDist = pow(clampedDist, falloff);
    vec4 colour = mix(endColour, startColour, falloffDist);

    imageStore(imgOut, pixel_coords, colour);
}
