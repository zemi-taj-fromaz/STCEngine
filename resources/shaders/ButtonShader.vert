#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec4 outColor;

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
    vec4 pos = model * vec4(inPosition.xy, 0.0, 1.0);

	
	float aspectRatio = resolution.res.x / resolution.res.y;
	
	//pos.x /= aspectRatio;

    gl_Position = vec4(pos.xy, 0.0, 1.0);
	
	outColor = objectBuffer.objects[gl_InstanceIndex].color;
	
	texCoord = inTexCoord;

}
