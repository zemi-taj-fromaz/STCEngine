#version 460

layout (location = 0) in vec3 textureCoord3D;

layout(set = 2, binding = 0) uniform samplerCube cubeSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(cubeSampler, textureCoord3D).rgb, 1.0);
}