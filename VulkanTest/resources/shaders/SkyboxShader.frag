#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 textureCoord3D;

layout(set = 0, binding = 1) uniform samplerCube cubeSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(cubeSampler, textureCoord3D);
}