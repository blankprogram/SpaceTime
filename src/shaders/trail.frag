#version 450 core

layout(location = 0) in float v_LifeFrac;
out vec4 FragColor;

uniform vec3 u_TrailColor; 
void main() {
    float alpha = clamp(v_LifeFrac, 0.0, 1.0);
    FragColor = vec4(u_TrailColor, alpha); 
}

