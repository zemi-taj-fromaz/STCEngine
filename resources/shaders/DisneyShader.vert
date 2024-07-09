#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 texCoord;
layout(location = 3) out vec3 wo;
layout(location = 4) out vec3 wi;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 pos;
} camera;

//push constants block

struct ObjectData
{
    mat4 model;
    vec4 color;
};


//all object matrices
layout(std140, set = 1, binding = 0) buffer ObjectBuffer{

	ObjectData objects[];
} objectBuffer;


layout(set = 2, binding = 0) uniform MousePosition{
	vec2 pos;
} mousePos;

void main() {
    mat4 model = objectBuffer.objects[gl_InstanceIndex].model;
    mat4 MVP =  camera.proj * camera.view * model;
    gl_Position =  MVP * vec4(inPosition, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    fragColor = objectBuffer.objects[gl_InstanceIndex].color.xyz;
    normal = normalize(normalMatrix * inNormal);
	
	vec2 xy = normalize(mousePos.pos);
	float z = -sqrt(1 - xy.x * xy.x - xy.y * xy.y);
	
	wi = normalize(vec3(-mousePos.pos.x, -mousePos.pos.y, -1.0));
	wo = vec4(camera.pos - model *  vec4(inPosition, 1.0)).xyz;
}