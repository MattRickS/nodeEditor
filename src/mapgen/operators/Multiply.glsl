#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

const int CHANNEL_RED   = 1;
const int CHANNEL_GREEN = 2;
const int CHANNEL_BLUE  = 4;
const int CHANNEL_ALPHA = 8;

uniform int channelMask = CHANNEL_RED | CHANNEL_GREEN | CHANNEL_BLUE | CHANNEL_ALPHA;
uniform float multiplier = 1.0f;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 value = imageLoad(imgIn, pixel_coords);
    if (bool(channelMask & CHANNEL_RED))   value.r *= multiplier;
    if (bool(channelMask & CHANNEL_GREEN)) value.g *= multiplier;
    if (bool(channelMask & CHANNEL_BLUE))  value.b *= multiplier;
    if (bool(channelMask & CHANNEL_ALPHA)) value.a *= multiplier;
    imageStore(imgOut, pixel_coords, value);
}
