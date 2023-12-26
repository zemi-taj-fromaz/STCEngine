#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout(location = 3) in vec4 particleColor;

layout(set = 2, binding = 0) uniform sampler2D tex1;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(tex1, texCoord);
	if(outColor.a < 0.1) discard;
	//outColor = particleColor;

    //outColor = vec4(fragColor + sceneData.ambientColor.xyz, 1.0f);
}