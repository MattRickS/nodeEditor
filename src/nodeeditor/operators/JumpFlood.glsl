#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn;
layout(rgba32f, binding=1) uniform image2D imgOut;

uniform int _iteration;
uniform int offset;

// Input is already converted to seeds, eg, heightmap >= 0.5 -> (x, y, 1, 1)
//                                          heightmap < 0.5 -> (0, 0, 0, 1)

bool isSeed(vec4 val)
{
    return val.x >= 0 && val.y >= 0;
}

ivec2 seedPos(vec4 val)
{
    return ivec2(val.xy);
}

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    vec4 value = imageLoad(imgIn, pixel);

    ivec2 closest;
    // Initialise min dist with value greater than any size the image could contain
    ivec2 imgSize = imageSize(imgIn);
    float minDist = dot(imgSize, imgSize);
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            vec4 val = imageLoad(imgIn, ivec2(pixel.x + x * offset, pixel.y + y * offset));
            if (isSeed(val))
            {
                ivec2 pos = seedPos(val);
                float dist = dot(pixel - pos, pixel - pos);
                if (dist < minDist)
                {
                    closest = pos;
                    minDist = dist;
                }
            }
        }
    }

    float trueDist = length(pixel - closest);
    vec4 outValue = vec4(closest, trueDist, (trueDist == 0) ? 0.0f : 1.0f);
    imageStore(imgOut, pixel, outValue);
}
