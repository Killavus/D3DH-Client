#version 330 core

out vec4 fragColor;
uniform sampler2D data;

in vec2 texCoord;

void main() {
  fragColor = texture(data, texCoord);
}

