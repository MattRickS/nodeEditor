#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgOut;
layout(std430, binding=0) buffer vectorsBlock
{
    vec2 data[];
} vectors;

uniform int divisionSize;

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(imgOut);
    int n = vectors.data.length();
    int bandSize = (size.y - divisionSize * (n - 1)) / n;
    int bandAndDivision = bandSize + divisionSize;

    int offset = pixel.y % bandAndDivision;
    int index = n - pixel.y / bandAndDivision - 1;
    vec4 colour = (offset < bandSize) ? vec4(vectors.data[index], 0.0f, 1.0f) : vec4(0);
    imageStore(imgOut, pixel, colour);
}
