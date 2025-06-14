#version 450 core

flat in uint textureIdx;

in float vert_lighting;
in vec3 vert_pos;
in vec2 vert_uv;

uniform sampler2DArray atlas;

out vec4 color;

void main() {
    vec4 tex = texture(atlas, vec3(vert_uv, textureIdx));
    
    // DEBUG: Show texture index as color gradient
    if (false) { // Change to true to enable debug
        float idx_norm = float(textureIdx) / 255.0;
        color = vec4(idx_norm, 0.0, 1.0 - idx_norm, 1.0);
        return;
    }
    
    // DEBUG: If texture is black/transparent, show red
    if (tex.rgb == vec3(0,0,0) || tex.a == 0) {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        color = vec4(tex.xyz * vert_lighting, tex.w);
    }
}