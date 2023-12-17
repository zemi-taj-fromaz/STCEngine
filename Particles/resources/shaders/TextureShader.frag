#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

layout(set = 1, binding = 0) uniform  SceneData{
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	vec4 sunPosition;
} sceneData;

layout(set = 3, binding = 0) uniform sampler2D tex1;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = texture(tex1, texCoord).xyz;
	outColor = vec4(color,1.0f);
    //outColor = vec4(fragColor + sceneData.ambientColor.xyz, 1.0f);
}