#version 430 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding=0) uniform image2D imgOutput;

void main()
{
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    imageStore(imgOutput, pixel_coords, vec4(pixel_coords, 0, 1));
}