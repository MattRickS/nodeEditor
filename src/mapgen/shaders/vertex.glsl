#version 430
layout (location=0) in vec3 inPosition;
layout (location=1) in vec2 inUV;

uniform mat4 transform;

out vec2 UV;

void main(void) {
    gl_Position = transform * vec4(inPosition,1.0);
    UV = inUV;
}