#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform ivec2 offset;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 offsetPos = ivec2(
        (pixel_coords + imageSize(imgIn) - offset) % imageSize(imgIn)
    );
    vec4 value = imageLoad(imgIn, offsetPos);
    imageStore(imgOut, pixel_coords, value);
}
