#version 330 core

out vec4 fragColor;
uniform sampler2DRect data;

in vec2 texCoord;

void main() {
  ivec2 uv = ivec2(texCoord.x, texCoord.y)
  grayColor = texelFetch(data, uv);

  fragColor = vec4(grayColor.x / 4500, grayColor.x / 4500, grayColor.x / 4500, 1.0);
}
