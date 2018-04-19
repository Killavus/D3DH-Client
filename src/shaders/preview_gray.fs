#version 330 core

out vec4 fragColor;
uniform sampler2D data;

in vec2 texCoord;

void main() {
  vec4 grayColor = texture(data, texCoord);

  fragColor = vec4(grayColor.r, grayColor.r, grayColor.r, 4500.0) / 4500.0;
}

