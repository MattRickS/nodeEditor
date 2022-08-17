#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D imgIn1;
layout(rgba32f, binding=1) uniform image2D imgIn2;
layout(rgba32f, binding=2) uniform image2D imgOut;

uniform bool red   = true;
uniform bool green = true;
uniform bool blue  = true;
uniform bool alpha = false;
uniform float multiplier = 1.0f;
uniform bool _ignoreImage1 = false;

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 val = imageLoad(imgIn1, pixel_coords);
    vec4 in2 = imageLoad(imgIn2, pixel_coords);
    if (red)   val.x *= multiplier;
    if (green) val.g *= multiplier;
    if (blue)  val.b *= multiplier;
    // TODO: Alpha never seems to be affected...
    if (alpha) val.a *= multiplier;
    if (!_ignoreImage1)
    {
        if (red)   val.r *= in2.r;
        if (green) val.g *= in2.g;
        if (blue)  val.b *= in2.b;
        if (alpha) val.a *= in2.a;
    }
    imageStore(imgOut, pixel_coords, val);
}
