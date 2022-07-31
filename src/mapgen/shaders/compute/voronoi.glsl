#version 430 core
layout(local_size_x = 8, local_size_y = 4) in;
layout(r32f, binding=0) uniform image2D heightmap;

uniform ivec2 offset = ivec2(0);
uniform float size = 100.0f;
uniform float skew = 0.5f;

vec3 hash3( vec2 vec ){
    vec3 q = vec3(
        dot(vec, vec2(127.1, 311.7)), 
        dot(vec, vec2(269.5, 183.3)), 
        dot(vec, vec2(419.2, 371.9))
    );
    return fract(sin(q) * 43758.5453);
}

vec3 voronoi(vec2 pos, float skew){
    vec2 cell = floor(pos);
    vec2 posInCell = fract(pos);

    float C = 100.0f;
    vec3 H = vec3(-1.0f);
    for( int j=-2; j<=2; j++ )
    {
        for( int i=-2; i<=2; i++ )
        {
            vec2 neighbourCell = vec2(float(i), float(j));
            vec3 hash = hash3(cell + neighbourCell);
            vec3 posInNeighbourCell = hash * vec3(skew, skew, 1.0);
            vec2 distanceToNeighbourPos = neighbourCell - posInCell + posInNeighbourCell.xy;
            float l = length(distanceToNeighbourPos);
            if (H.x < 0 || l < C)
            {
                C = l;
                H = hash;
            }
        }
    }
    
    return H;
}

void main() {
    vec3 noise = voronoi((gl_GlobalInvocationID.xy + offset) / size, skew);
    imageStore(heightmap, ivec2(gl_GlobalInvocationID.xy), vec4(noise.x, noise.x, noise.x, 1));
}