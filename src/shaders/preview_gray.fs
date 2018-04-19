#version 330 core

out vec4 fragColor;
uniform sampler2D data;

in vec2 texCoord;

void main() {
  ivec2 uv = ivec2(texCoord.x, texCoord.y);
  vec4 grayColor = texelFetch(data, uv, 0);

  fragColor = vec4(grayColor.r, grayColor.r, grayColor.r, 4500.0) / 4500.0;
}

