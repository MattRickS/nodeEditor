#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn1;
layout(rgba32f, binding=1) uniform image2D imgIn2;
layout(rgba32f, binding=2) uniform image2D imgOut;

uniform bool red   = true;
uniform bool green = true;
uniform bool blue  = true;
uniform bool alpha = false;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 in1 = imageLoad(imgIn1, pixel_coords);
    vec4 in2 = imageLoad(imgIn2, pixel_coords);
    if (!red)   in2.r = 0;
    if (!green) in2.g = 0;
    if (!blue)  in2.b = 0;
    if (!alpha) in2.a = 0;
    // TODO: Channel control to prevent adding alpha
    imageStore(imgOut, pixel_coords, in1 + in2);
}
