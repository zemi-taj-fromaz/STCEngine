#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

layout(set = 2, binding = 0, rgba32f) uniform readonly image2D heightmap;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 color = imageLoad(heightmap, ivec2(texCoord));
	//if(color == vec3(0.0, 0.0, 0.0)) discard;
	outColor = vec4(color.x, color.y, 0.0f,1.0f);
    //outColor = vec4(fragColor, 1.0f);
}