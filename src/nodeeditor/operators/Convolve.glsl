#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;
layout(std430, binding=2) buffer Kernel
{
    int width;
    int height;
    float distribution[];
} kernel;

const int CHANNEL_RED   = 1;
const int CHANNEL_GREEN = 2;
const int CHANNEL_BLUE  = 4;
const int CHANNEL_ALPHA = 8;

uniform int channelMask = CHANNEL_RED | CHANNEL_GREEN | CHANNEL_BLUE | CHANNEL_ALPHA;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 offset = ivec2(kernel.width / 2, kernel.height / 2);

    vec4 value = vec4(0);
    for (int y = 0; y < kernel.height; ++y)
    {
        for (int x = 0; x < kernel.width; ++x)
        {
            value += imageLoad(imgIn, pixel_coords + ivec2(x, y) - offset) * kernel.distribution[y * kernel.width + x];
        }
    }

    vec4 currentValue = imageLoad(imgIn, pixel_coords);
    if (!bool(channelMask & CHANNEL_RED))   value.r = currentValue.r;
    if (!bool(channelMask & CHANNEL_GREEN)) value.g = currentValue.g;
    if (!bool(channelMask & CHANNEL_BLUE))  value.b = currentValue.b;
    if (!bool(channelMask & CHANNEL_ALPHA)) value.a = currentValue.a;

    imageStore(imgOut, pixel_coords, value);
}
