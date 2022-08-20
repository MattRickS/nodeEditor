#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn0;
layout(rgba32f, binding=1) uniform image2D imgIn1;
layout(rgba32f, binding=2) uniform image2D imgIn2;
layout(rgba32f, binding=3) uniform image2D imgOut;

uniform float blend = 1.0f;
uniform bool _ignoreImage2 = false;

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    float mask = 1;
    if (!_ignoreImage2)
    {
        mask = imageLoad(imgIn2, pixel).a;
    }

    vec4 A = imageLoad(imgIn0, pixel);
    vec4 B = imageLoad(imgIn1, pixel);

    vec4 value = B + A * (1 - B.a);

    imageStore(imgOut, pixel, mix(A, value, blend * mask));
}
