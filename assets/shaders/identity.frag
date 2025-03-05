#version 450 core
in vec2 uv;

uniform sampler2D colorTexture;
layout(location = 0) out vec4 color;

void main() {
	color = texture(colorTexture, uv);
}
