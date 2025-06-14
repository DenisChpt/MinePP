#version 450 core

// BlockVertex optimisé : 6 octets envoyés comme 3 uint16_t
layout(location = 0) in uint data_01;   // bytes 0-1
layout(location = 1) in uint data_23;   // bytes 2-3  
layout(location = 2) in uint data_45;   // bytes 4-5

uniform mat4 MVP;
uniform vec3 lightDirection;
uniform uint textureAnimation;

flat out uint textureIdx;

out float vert_lighting;
out vec3 vert_pos;
out vec2 vert_uv;

void main() {
    // Extract data from bytes 0-1
    // Byte 0: x(5 bits) + z_low(3 bits)
    // Byte 1: z_high(2 bits) + u(1 bit) + v(1 bit) + spare(4 bits)
    uint byte0 = data_01 & 0xFF;
    uint byte1 = (data_01 >> 8) & 0xFF;
    
    uint xPos = byte0 & 0x1F;                          // bits 0-4 of byte 0
    uint z_low = (byte0 >> 5) & 0x07;                 // bits 5-7 of byte 0
    uint z_high = byte1 & 0x03;                       // bits 0-1 of byte 1
    uint zPos = z_low | (z_high << 3);                // combine z parts
    
    uint xUv = (byte1 >> 2) & 0x01;                   // bit 2 of byte 1
    uint yUv = (byte1 >> 3) & 0x01;                   // bit 3 of byte 1
    
    // Extract data from bytes 2-3
    // Byte 2: y(8 bits)
    // Byte 3: textureIndex(8 bits)
    uint yPos = data_23 & 0xFF;                       // byte 2
    uint baseTextureIdx = (data_23 >> 8) & 0xFF;      // byte 3
    
    // Extract data from bytes 4-5
    // Byte 4: occlusion(2 bits) + animated(1 bit) + spare(5 bits)
    uint byte4 = data_45 & 0xFF;
    uint occlusionLevel = byte4 & 0x03;               // bits 0-1 of byte 4
    uint animated = (byte4 >> 2) & 0x01;              // bit 2 of byte 4
    
    // Calculate final values
    vert_pos = vec3(xPos, yPos, zPos);
    vert_uv = vec2(xUv, yUv);
    textureIdx = baseTextureIdx + textureAnimation * animated;
    vert_lighting = 0.75f + 0.08f * occlusionLevel;
    
    gl_Position = MVP * vec4(vert_pos, 1);
}