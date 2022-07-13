#version 330 core
const vec2 quad_vertices[4] = vec2[4]( vec2( -1.0, -1.0), vec2( 1.0, -1.0), vec2( -1.0, 1.0), vec2( 1.0, 1.0));
const vec2 quad_uvs[4] = vec2[4]( vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0));
out vec2 UV;

uniform mat4 transform = mat4(1.0f);

void main()
{
    UV = quad_uvs[gl_VertexID];
    gl_Position = transform * vec4(quad_vertices[gl_VertexID], 0.0, 1.0);
}