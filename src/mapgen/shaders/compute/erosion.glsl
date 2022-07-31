#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(r32f, binding=0) uniform image2D heightmap;
layout(rgba32f, binding=1) uniform image2D outflowmap;

uniform float rain = 0.01f;

/*
Calculates the gradient between the four closest pixels, and height of pos.

Example:

2 +-----+-----+-----+
  |     |     |     |
  |  A  |  B  |  C  |
  |    x|     |z    |
1 +-----+-----+-----+
  |     |    y|     |
  |  D  |  E  |  F  |
  |     |     |     |
0 +-----+-----+-----+
  0     1     2     3

Pos x = (0.75,1.25) has pixel offset ( 0.25,-0.25) so uses SW=D, SE=E, NW=A, NE=B with weighted offset (0.25,0.75)
Pos y = (1.75,0.75) has pixel offset ( 0.25, 0.25) so uses SW=E, SE=F, NW=B, NE=C with weighted offset (0.25,0.25)
Pos z = (2.25,1.25) has pixel offset (-0.25,-0.25) so uses SW=E, SE=F, NW=B, NE=C with weighted offset (0.75,0.75)

*/
vec3 calcGradient(vec2 pos)
{
    ivec2 pixel = ivec2(pos);
    vec2 offset = pos - (pixel + 0.5f);  // offset from middle of pixel

    // Determine which quadrant of the pixel the pos is in for selecting nearest pixels
    int offsetW = 0, offsetE = 1, offsetS = 0, offsetN = 1;
    if (offset.x < 0) { offset.x += 1; offsetW = -1; offsetE = 0; }
    if (offset.y < 0) { offset.y += 1; offsetS = -1; offsetN = 0; }

    // Reading from a pixel outside the image will return all 0s. Should account for it in future
    vec4 SW = imageLoad(heightmap, ivec2(pixel.x + offsetW, pixel.y + offsetS));
    vec4 SE = imageLoad(heightmap, ivec2(pixel.x + offsetE, pixel.y + offsetS));
    vec4 NE = imageLoad(heightmap, ivec2(pixel.x + offsetE, pixel.y + offsetN));
    vec4 NW = imageLoad(heightmap, ivec2(pixel.x + offsetW, pixel.y + offsetN));

    // bilinear interpolation of the positions height
    float height = SW.x * (1 - offset.x) * (1 - offset.y) + SE.x * offset.x * (1 - offset.y) + NW.x * (1 - offset.x) * offset.y + NE.x * offset.x * offset.y;

    // bilinear gradients at the position, gradient from highest to lowest
    float gradientX = (NW.x - NE.x) * (1 - offset.y) + (SW.x - SE.x) * offset.y;
    float gradientY = (SW.x - NW.x) * (1 - offset.x) + (SE.x - NE.x) * offset.x;

    return vec3(gradientX, gradientY, height);
}

/*
Determines the multiplier for dir to reach the next axis intersection from pos.

Solves the parametric line equation y = mx + b where y % 1 == 0 for both axis
and takes the smallest value for b.

Example:

(0, 1)        (1, 1)
  +-------x---+
  | . . ./. . |
  | . . / . . |
  | . ./. . . |
  | . p . . . |
  | . . . . . |
  +-----------+
(0, 0)        (1, 0)

p = (0.33, 0.33)
d = (0.16, 0.33)
nextAxisIntersection = (1, 1)
nextAxisDelta = (4.1875, 2.0303)
delta = 2.0303

x = p + d * delta = (0.6548, 1)

*/
float calcDelta(vec2 pos, vec2 dir)
{
    // Add/Subtract 1 to move into next integer bound before floor in case the current pos
    // is on a boundary (ie, pos.x=1.0f, dir.x=0.1f, nextAxisIntersection.x=2.0f not 1.0f)
    vec2 nextAxisIntersection = floor(vec2(
        pos.x + (dir.x > 0 ? 1 : - 1),
        pos.y + (dir.y > 0 ? 1 : - 1)
    ));
    vec2 nextAxisDelta = abs((nextAxisIntersection - pos) / dir);
    return min(nextAxisDelta.x, nextAxisDelta.y);
}

void main()
{
    float water = 0.0f;

    vec2 pos = gl_GlobalInvocationID.xy + 0.5f;
    vec3 grad = calcGradient(pos);
    vec2 dir = grad.xy;
    // If there's no gradient, there's no more movement - stop processing.
    if (dir.x == 0 && dir.y == 0)
    {
        // TODO: Accrue stopped water into a "pooled" water map.
        //       This could potentially be the same as the vector map, but the vector is 0

        return;
    }
    // TODO: inertia from previous movement, possibly even use a vector map for water flow
    // so that particles tend to flow together.

    float delta = calcDelta(pos, dir);
    vec2 nextPos = pos + dir * delta;

    float heightDiff = grad.z - calcGradient(nextPos).z;

    imageStore(heightmap, ivec2(gl_GlobalInvocationID.xy), vec4(1, 0, 0, 1));
    imageStore(outflowmap, ivec2(gl_GlobalInvocationID.xy), vec4(dir, grad.z, 1));
}
