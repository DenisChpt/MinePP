#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform float time;
uniform bool invertColors;
uniform bool grayscale;
uniform bool sharpen;
uniform bool blur;

const float offset = 1.0 / 300.0;  // Ajuster en fonction de la résolution

vec3 applyGrayscale(vec3 color) {
	float avg = (color.r + color.g + color.b) / 3.0;
	return vec3(avg);
}

vec3 applyInversion(vec3 color) {
	return vec3(1.0 - color);
}

vec3 applySharpen() {
	vec3 col = texture(sceneTexture, TexCoords).rgb;
	vec3 surrounding[9];
	surrounding[0] = texture(sceneTexture, TexCoords + vec2(-offset,  offset)).rgb;
	surrounding[1] = texture(sceneTexture, TexCoords + vec2( 0.0,    offset)).rgb;
	surrounding[2] = texture(sceneTexture, TexCoords + vec2( offset,  offset)).rgb;
	surrounding[3] = texture(sceneTexture, TexCoords + vec2(-offset,  0.0)).rgb;
	surrounding[4] = col;
	surrounding[5] = texture(sceneTexture, TexCoords + vec2( offset,  0.0)).rgb;
	surrounding[6] = texture(sceneTexture, TexCoords + vec2(-offset, -offset)).rgb;
	surrounding[7] = texture(sceneTexture, TexCoords + vec2( 0.0,   -offset)).rgb;
	surrounding[8] = texture(sceneTexture, TexCoords + vec2( offset, -offset)).rgb;

	mat3 kernel = mat3(
		-1, -1, -1,
		-1,  9, -1,
		-1, -1, -1
	);

	vec3 result = vec3(0.0);
	for (int i = 0; i < 9; i++) {
		result += surrounding[i] * kernel[i / 3][i % 3];
	}

	return result;
}

vec3 applyBlur() {
	vec3 col = vec3(0.0);
	float kernel[9] = float[]( 1.0 / 16, 2.0 / 16, 1.0 / 16,
							   2.0 / 16, 4.0 / 16, 2.0 / 16,
							   1.0 / 16, 2.0 / 16, 1.0 / 16 );

	vec3 surrounding[9];
	surrounding[0] = texture(sceneTexture, TexCoords + vec2(-offset,  offset)).rgb;
	surrounding[1] = texture(sceneTexture, TexCoords + vec2( 0.0,    offset)).rgb;
	surrounding[2] = texture(sceneTexture, TexCoords + vec2( offset,  offset)).rgb;
	surrounding[3] = texture(sceneTexture, TexCoords + vec2(-offset,  0.0)).rgb;
	surrounding[4] = texture(sceneTexture, TexCoords).rgb;
	surrounding[5] = texture(sceneTexture, TexCoords + vec2( offset,  0.0)).rgb;
	surrounding[6] = texture(sceneTexture, TexCoords + vec2(-offset, -offset)).rgb;
	surrounding[7] = texture(sceneTexture, TexCoords + vec2( 0.0,   -offset)).rgb;
	surrounding[8] = texture(sceneTexture, TexCoords + vec2( offset, -offset)).rgb;

	for (int i = 0; i < 9; i++) {
		col += surrounding[i] * kernel[i];
	}

	return col;
}

void main() {
	vec3 color = texture(sceneTexture, TexCoords).rgb;

	if (grayscale) {
		color = applyGrayscale(color);
	}

	if (invertColors) {
		color = applyInversion(color);
	}

	if (sharpen) {
		color = applySharpen();
	}

	if (blur) {
		color = applyBlur();
	}

	FragColor = vec4(color, 1.0);
}
