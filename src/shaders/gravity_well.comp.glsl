#version 450
layout(local_size_x=8, local_size_y=8) in;

struct Body { vec3 pos; float mass; };
layout(std140, binding=0) readonly buffer Bodies {
  Body bodies[];
};

layout(std430, binding=1) buffer Heights {
  vec4 vertices[]; // x,y,z,unused
};

uniform float u_size;
uniform int   u_res;
uniform float u_G;

void main(){
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  int N = 2*u_res + 1;
  if(gid.x >= N || gid.y >= N) return;
  int idx = (gid.y*(N-1) + gid.x)*2; // line‐grid layout...
  float step = u_size / float(u_res);
  float x = (float(gid.x)-u_res)*step;
  float z = (float(gid.y)-u_res)*step;
  float y = 0.0;
  for(int i=0;i<bodies.length();++i){
    Body b = bodies[i];
    vec2 diff = vec2(x,z) - b.pos.xz;
    float d = length(diff) + 0.1;
    y += -u_G * b.mass / d;
  }
  vertices[idx+0] = vec4(x, y, z, 0);
  vertices[idx+1] = vec4(x+step, /*…compute neighbor…*/, 0);
}
