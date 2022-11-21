#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgOut;

uniform vec4 color = vec4(0);

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    imageStore(imgOut, pixel_coords, color);
}
