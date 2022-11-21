#version 330 core
in vec2 UV;
out vec4 color;

uniform sampler2D renderTexture;
uniform int isolateChannel = -1;

void main(){
    vec4 col = texture(renderTexture, UV);
    if (isolateChannel >=0) col = vec4(col[isolateChannel], col[isolateChannel], col[isolateChannel], 1);
    color = col;
}