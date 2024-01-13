#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

layout(set = 2, binding = 0) uniform sampler2D tex1;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = texture(tex1, texCoord).xyz;
	if(color == vec3(0.0, 0.0, 0.0)) discard;
	outColor = vec4(color,1.0f);
    //outColor = vec4(fragColor, 1.0f);
}