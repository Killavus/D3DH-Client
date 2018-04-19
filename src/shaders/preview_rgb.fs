#version 330 core

out vec4 fragColor;
uniform sampler2D data;

in vec2 texCoord;

void main() {
  ivec2 uv = ivec2(texCoord.x, texCoord.y);
  fragColor = texelFetch(data, uv, 0);
}

