#version 330 core
in vec2 UV;
out vec4 color;

uniform sampler2D inImage;

void main(){
    vec4 col = 1 - texture(inImage, UV);
    color = vec4(col.xyz, 1);
}
