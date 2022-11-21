#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform float scale;
uniform int channel;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(imgIn);

    float right = imageLoad(imgIn, ivec2(min(pixel_coords.x + 1, size.x - 1), pixel_coords.y))[channel];
    float left = imageLoad(imgIn, ivec2(max(pixel_coords.x - 1, 0), pixel_coords.y))[channel];
    float bottom = imageLoad(imgIn, ivec2(pixel_coords.x, max(pixel_coords.y - 1, 0)))[channel];
    float top = imageLoad(imgIn, ivec2(pixel_coords.x, min(pixel_coords.y + 1, size.y - 1)))[channel];

    vec3 normals = -0.25f * vec3(
        2.0f * scale * (right - left),
        2.0f * scale * (bottom - top),
        -4.0f
    );
    imageStore(imgOut, pixel_coords, vec4(normalize(normals), 1));
}
