#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D heightmapImg;
layout(rgba32f, binding=1) uniform image2D waterDistImg;
layout(rgba32f, binding=2) uniform image2D imgOut;

uniform int loBand;
uniform int hiBand;
uniform float falloff;
uniform float heightMult;
uniform float waterMult;

uniform bool _ignoreImage1;

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    vec2 halfSize = imageSize(heightmapImg) * 0.5f;
    float dist = (abs(halfSize.y - pixel.y) - hiBand) / (halfSize.y - hiBand - loBand);
    
    float temp = pow( max(min(1 - dist, 1), 0), falloff);
    temp = max(0, temp - heightMult * max(0, imageLoad(heightmapImg, pixel).x));
    if (!_ignoreImage1)
    {
        temp = max(0, temp - waterMult * imageLoad(waterDistImg, pixel).x);
    }
    imageStore(imgOut, pixel, vec4(temp, temp, temp, 1));
}
