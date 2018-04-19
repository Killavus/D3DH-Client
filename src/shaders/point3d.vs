#version 330 core
layout (location = 0) in vec4 pos;

uniform mat4 projection;
uniform mat4 camera;

void main() {
  gl_Position = projection * camera * pos;
}
