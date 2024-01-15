#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 outColor;

layout (set = 2, binding = 0) uniform sampler2D tex;

void main() {

	outColor = vec4(texture(tex, texCoord).xyz, 1.0);
	//outColor = color;
	//outColor = vec4(1.0, 1.0, 1.0, 1.0);

}
