#version 450 core

uniform sampler2D colorTexture;
uniform int screenHeight;

layout(location = 0) out vec4 color;

void main() {
    int flippedY = (screenHeight - 1) - int(gl_FragCoord.y);

    color = texelFetch(colorTexture, ivec2(gl_FragCoord.x, flippedY), 0);
}
