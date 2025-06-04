#version 450 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    FragColor = texture(u_Texture, TexCoord);
}

