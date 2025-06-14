#version 450 core

flat in uint textureIndex;

in float vert_lighting;
in vec3 vert_pos;
in vec2 vert_uv;

uniform sampler2DArray atlas;

out vec4 color;

void main() {
    vec4 tex = texture(atlas, vec3(vert_uv, textureIndex));
    
    // DEBUG: If texture is black/transparent, show red
    if (tex.rgb == vec3(0,0,0) || tex.a == 0) {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        color = vec4(tex.xyz * vert_lighting, tex.w);
    }
}