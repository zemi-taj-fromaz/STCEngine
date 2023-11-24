#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 textureCoord3D;

layout(set = 0, binding = 1) uniform  SceneData{
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	vec4 sunPosition;
} sceneData;


layout(set = 1, binding = 0) uniform samplerCube cubeSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(cubeSampler, textureCoord3D).rgb, 1.0);
	gl_FragDepth = 1.0;
}