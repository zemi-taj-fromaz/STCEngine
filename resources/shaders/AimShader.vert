#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 fragColor;

struct ObjectData
{
    mat4 model;
    vec4 color;
};

//all object matrices
layout(std140, set = 0, binding = 0) buffer ObjectBuffer{

	ObjectData objects[];
} objectBuffer;

layout(set = 1, binding = 0) uniform Resolution{
	vec2 res;
} resolution;

void main() {
    mat4 model = objectBuffer.objects[gl_InstanceIndex].model;
    vec4 pos = model * vec4(inPosition, 1.0);
	
	float aspectRatio = resolution.res.x / resolution.res.y;
	
	pos.x /= aspectRatio;

    gl_Position = vec4(pos.xy, 0.0, 1.0);
    fragColor = objectBuffer.objects[gl_InstanceIndex].color;
}
