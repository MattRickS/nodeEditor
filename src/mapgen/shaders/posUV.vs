#version 430
layout (location=0) in vec3 inPosition;
layout (location=1) in vec2 inUV;

uniform mat4 model = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 projection = mat4(1);

out vec2 UV;

void main(void) {
    gl_Position = projection * view * model * vec4(inPosition,1.0);
    UV = inUV;
}