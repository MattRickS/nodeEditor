#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform float minValue;
uniform float maxValue;

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    vec4 value = imageLoad(imgIn, pixel);

    for (int i = 0; i < 3; ++i)
    {
        if (value[i] < minValue)
        {
            value[i] = minValue;
        }
        else if (value[i] > maxValue)
        {
            value[i] = maxValue;
        }
    }
    imageStore(imgOut, pixel, value);
}
