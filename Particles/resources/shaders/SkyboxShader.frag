#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 textureCoord3D;

layout(set = 1, binding = 0) uniform samplerCube cubeSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(cubeSampler, textureCoord3D).rgb, 1.0);
	gl_FragDepth = 1.0;
}