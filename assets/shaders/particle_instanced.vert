#version 450 core

// Per-vertex attributes
layout(location = 0) in vec3 position;

// Per-instance attributes
layout(location = 1) in mat4 instanceTransform;  // Instance transform matrix (locations 1-4)
layout(location = 5) in vec4 instanceColor;      // Instance color

// Output to fragment shader
out vec4 fragColor;

void main() {
    // Transform vertex position with instance transform
    gl_Position = instanceTransform * vec4(position, 1.0);
    
    // Pass color to fragment shader
    fragColor = instanceColor;
}