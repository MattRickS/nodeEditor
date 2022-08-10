#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn1;
layout(rgba32f, binding=0) uniform image2D imgIn2;
layout(rgba32f, binding=1) uniform image2D imgOut;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 val = imageLoad(imgIn1, pixel_coords) + imageLoad(imgIn2, pixel_coords);
    // TODO: Channel control to prevent adding alpha
    imageStore(imgOut, pixel_coords, val);
}
