#version 450 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in float a_LifeFrac;
layout(location = 0) out float v_LifeFrac;
uniform mat4 u_MVP;

void main() {
    v_LifeFrac  = a_LifeFrac;
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
}

