#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn1;
layout(rgba32f, binding=1) uniform image2D imgIn2;
layout(rgba32f, binding=2) uniform image2D imgOut;

uniform bool red   = true;
uniform bool green = true;
uniform bool blue  = true;
uniform bool alpha = false;
uniform float value = 0.0f;
uniform bool _ignoreImage1 = false;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 in1 = imageLoad(imgIn1, pixel_coords);
    if (red)   in1.r += value;
    if (green) in1.g += value;
    if (blue)  in1.b += value;
    if (alpha) in1.a += value;
    if (!_ignoreImage1)
    {
        vec4 in2 = imageLoad(imgIn2, pixel_coords);
        if (red)   in1.r = in2.r;
        if (green) in1.g = in2.g;
        if (blue)  in1.b = in2.b;
        if (alpha) in1.a = in2.a;
    }
    imageStore(imgOut, pixel_coords, in1);
}
