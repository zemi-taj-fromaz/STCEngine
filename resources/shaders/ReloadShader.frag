#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragPos;
layout(location = 2) in vec3 position;

layout(location = 0) out vec4 outColor;

layout (set = 2, binding = 0) uniform ParameterUBO {
    float reloadTime;
} ubo;

void main() {

	if((position.x * position.x + position.y * position.y > 0.5 * 0.5) ||	(position.x * position.x + position.y * position.y < 0.3 * 0.3)) discard;
		
    outColor = fragColor;
}
