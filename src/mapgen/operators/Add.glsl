#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform bool red   = true;
uniform bool green = true;
uniform bool blue  = true;
uniform bool alpha = false;
uniform float add = 0.0f;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 value = imageLoad(imgIn, pixel_coords);
    if (red)   value.r += add;
    if (green) value.g += add;
    if (blue)  value.b += add;
    if (alpha) value.a += add;
    imageStore(imgOut, pixel_coords, value);
}
