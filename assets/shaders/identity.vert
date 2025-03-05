#version 450 core
layout(location = 0) in vec3 position;

out vec2 uv;

void main() {
	// position ∈ [-1,+1], on la convertit en [0,1] pour uv
	uv = (position.xy * 0.5) + 0.5;
	// Si l’image est à l’envers, on inverse uv.y
	uv.y = 1.0 - uv.y;

	gl_Position = vec4(position, 1.0);
}