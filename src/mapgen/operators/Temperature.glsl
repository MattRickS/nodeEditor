#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(rgba32f, binding=0) uniform image2D heightmapImg;
layout(rgba32f, binding=1) uniform image2D waterDistImg;
layout(rgba32f, binding=2) uniform image2D imgOut;

uniform float loFalloff;
uniform float hiFalloff;
uniform float heightMult;
uniform float waterMult;

uniform bool _ignoreImage1;

vec2 cubic_bezier(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t)
{
    return pow(1 - t, 3) * p0 + 3 * pow(1 - t, 2) * t * p1 + 3 * (1 - t) * t * t * p2 + t * t * t * p3;
}

void main(){
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    vec2 halfSize = imageSize(heightmapImg) * 0.5f;

    // y-offset from center, 1 at center, 0 at edge
    float y = 1.0f - abs(halfSize.y - pixel.y) / halfSize.y;
    float tolerance = 0.5f / halfSize.y;  // within half a pixel

    // Drawing a cubic curve vertically from middle of the image to the edge.
    // p(x, y), where x is the vertical axis, ie, offset from center, and y is
    // the target value at that position. Here we're getting curve from 0 to 1.
    vec2 p0 = vec2(0, 0);
    vec2 p1 = vec2(loFalloff, 0);
    vec2 p2 = vec2(1 - hiFalloff, 1);
    vec2 p3 = vec2(1, 1);

    // Bezier requires t, an offset along the curve but we need to know the
    // value at the pixel position. Binary search until p.x is within half a
    // pixel of the target pixel.
    float lower = 0.0f;
    float upper = 1.0f;
    float t = 0.5f;
    vec2 p = cubic_bezier(p0, p1, p2, p3, t);
    int i = 20;  // Safety check to prevent broken curves locking up the shader
    while (abs(p.x - y) > tolerance && i-- > 0)
    {
        if (p.x > y)
        {
            upper = t;
        }
        else
        {
            lower = t;
        }
        t = lower + (upper - lower) * 0.5f;
        p = cubic_bezier(p0, p1, p2, p3, t);
    }

    // The temperature is the curve value at this pixel
    float temp = p.y;
    // Temperature gets colder the higher the elevation
    temp = max(0, temp - heightMult * max(0, imageLoad(heightmapImg, pixel).x));
    if (!_ignoreImage1)
    {
        // Temperature gets colder the further from land
        temp = max(0, temp - waterMult * imageLoad(waterDistImg, pixel).x);
    }
    imageStore(imgOut, pixel, vec4(temp, temp, temp, 1));
}
