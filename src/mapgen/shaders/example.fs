#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(gl_FragCoord.x, gl_FragCoord.y, 0, 1);
}
