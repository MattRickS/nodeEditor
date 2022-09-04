#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgOut;

uniform uint size = 128;
uniform vec4 color1 = vec4(0);
uniform vec4 color2 = vec4(1);

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    uvec2 ratio = (pixel_coords / size) % 2;
    vec4 color = bool(ratio.x ^ ratio.y) ? color1 : color2;
    imageStore(imgOut, pixel_coords, color);
}
