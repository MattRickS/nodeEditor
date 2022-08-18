#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    float val = 1 - imageLoad(imgIn, pixel_coords).x;
    imageStore(imgOut, pixel_coords, vec4(val, val, val, 1));
}
