#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 texCoord;


layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 pos;
} camera;

layout(set = 1, binding = 0) uniform BufferObject {
	float deltaTime;
} bo;


//push constants block

struct ObjectData
{
    mat4 model;
    vec4 color;
};

//all object matrices
layout(std140, set = 2, binding = 0) buffer ObjectBuffer{

	ObjectData objects[];
} objectBuffer;

layout(set = 3, binding = 0) uniform sampler2D text;


vec2 multiplyComplex(vec2 c1, vec2 c2) {
    vec2 result;
    result.x = c1.x * c2.x - c1.y * c2.y;
    result.y = c1.x * c2.y + c1.y * c2.x;
    return result;
}


void main() {
    mat4 model = objectBuffer.objects[gl_InstanceIndex].model;
    mat4 MVP =  camera.proj * camera.view * model;

	vec4 tex = texture(text, inTexCoord);
	
	vec4 position = vec4(inPosition.x, tex.x, inPosition.z, 1.0);
	//vec4 position = vec4(inPosition, 1.0);
	gl_Position =  MVP * position;
    texCoord = inTexCoord;
    fragColor = objectBuffer.objects[gl_InstanceIndex].color.xyz;
    normal = inNormal;
}