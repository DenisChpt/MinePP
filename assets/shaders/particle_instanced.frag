#version 450 core

// Input from vertex shader
in vec4 fragColor;

// Output
out vec4 outColor;

void main() {
    outColor = fragColor;
    
    // Discard fully transparent fragments
    if (outColor.a < 0.01) {
        discard;
    }
}