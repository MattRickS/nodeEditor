#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgOut;
layout(std430, binding=0) buffer bandsBlock
{
    vec4 data[];
} bands;

uniform int divisionSize;
uniform vec4 divisionColour;

// XXX: Not working currently
void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(imgOut);
    int n = bands.data.length();
    int bandSize = (size.y - divisionSize * (n - 1)) / n;
    int bandAndDivision = bandSize + divisionSize;

    int offset = pixel.y % bandAndDivision;
    int index = n - pixel.y / bandAndDivision - 1;
    vec4 colour = (offset < bandSize) ? bands.data[index] : divisionColour;
    // imageStore(imgOut, pixel, colour);
    imageStore(imgOut, pixel, vec4(float(n), bandSize, float(index), 1.0f));
}
