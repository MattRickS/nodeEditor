#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform int red;
uniform int green;
uniform int blue;
uniform int alpha;
uniform int white;
uniform int black;

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    vec4 inValue = imageLoad(imgIn, pixel);
    vec4 outValue;

    int arr[6] = int[6](red, green, blue, alpha, white, black);
    for (int i = 0; i < 6; ++i)
    {
        for (int c = 0; c < 4; ++c)
        {
            if (bool(arr[i] & (1 << c)))
            {
                // RGBA
                if (i < 4)
                {
                    outValue[c] = inValue[i];
                }
                // Fixed white and black values
                else if (i == 4)
                {
                    outValue[c] = 1.0f;
                }
                else
                {
                    outValue[c] = 0.0f;
                }
            }
        }
    }

    imageStore(imgOut, pixel, outValue);
}
