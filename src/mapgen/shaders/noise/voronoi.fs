#version 330 core
in vec2 UV;
out vec4 color;

uniform ivec2 offset = ivec2(0);
uniform float size = 100.0f;
uniform float skew = 0.5f;
uniform float blend = 0.0f;

vec3 hash3( vec2 vec ){
    vec3 q = vec3(
        dot(vec, vec2(127.1, 311.7)), 
        dot(vec, vec2(269.5, 183.3)), 
        dot(vec, vec2(419.2, 371.9))
    );
    return fract(sin(q) * 43758.5453);
}

vec3 voronoi(vec2 pos, float skew, float blend){
    vec2 cell = floor(pos);
    vec2 posInCell = fract(pos);
    
    // Curved range from 1-64 based on blend (1=1, 0=64)
    // float strength = 1.0 + 63.0 * pow(1.0 - blend, 4.0);

    // float color = 0.0;
    // float totalWeight = 0.0;

    float C = 100.0f;
    vec3 H = vec3(-1.0f);
    for( int j=-2; j<=2; j++ )
    {
        for( int i=-2; i<=2; i++ )
        {
            // Bubble cells
            // vec2 neighbourCell = vec2(float(i), float(j));
            // vec3 posInNeighbourCell = hash3(cell + neighbourCell) * vec3(skew, skew, 1.0);
            // vec2 distanceToNeighbourPos = neighbourCell - posInCell + posInNeighbourCell.xy;
            // C = min(C, length(distanceToNeighbourPos));

            // Coloured cells
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

            // Grayscale blended cells
            // vec2 neighbourCell = vec2(float(i), float(j));
            // vec3 posInNeighbourCell = hash3(cell + neighbourCell) * vec3(skew, skew, 1.0);
            // vec2 distanceToNeighbourPos = neighbourCell - posInCell + posInNeighbourCell.xy;
            // float weight = pow(1.0 - smoothstep(0.0, 1.414, length(distanceToNeighbourPos)), strength);
            // color += posInNeighbourCell.z * weight;
            // totalWeight += weight;
        }
    }
    
    return H;
    // return 1-C;
    // return color / totalWeight;
}

void main() {
    // float noise = voronoi((gl_FragCoord.xy + offset) / size, skew, blend);
    // color = vec4(noise, noise, noise, 1);
    vec3 noise = voronoi((gl_FragCoord.xy + offset) / size, skew, blend);
    color = vec4(noise, 1);
}