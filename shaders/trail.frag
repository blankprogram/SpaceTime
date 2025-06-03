
#version 450 core

layout(location = 0) in float v_LifeFrac; 
layout(location = 0) out vec4 FragColor;

void main() {
    float alpha = clamp(v_LifeFrac, 0.0, 1.0);
    FragColor = vec4(1.0, 1.0, 1.0, alpha); // white, fading by lifeFrac
}

