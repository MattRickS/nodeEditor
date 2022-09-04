#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform uint loBand;
uniform uint hiBand;
uniform float falloff;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 halfSize = imageSize(imgIn) * 0.5f;
    float dist = (abs(halfSize.y - pixel_coords.y) - hiBand) / (halfSize.y - hiBand - loBand);
    float val = pow( max(min(1 - dist, 1), 0), falloff);
    float temp = max(0, val - max(0, imageLoad(imgIn, pixel_coords).x));
    imageStore(imgOut, pixel_coords, vec4(val, val, val, 1));
}
