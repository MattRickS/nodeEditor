#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform int channel;
uniform float minValue;
uniform float maxValue;

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    vec4 value = imageLoad(imgIn, pixel);

    if (value[channel] >= minValue && value[channel] <= maxValue)
    {
        value = vec4(pixel.xy, 0.0f, 1.0f);
    }
    else
    {
        value = vec4(-1, -1, -1, 0);
    }
    imageStore(imgOut, pixel, value);
}
