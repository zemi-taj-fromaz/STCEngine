#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec2 texCoord;
layout(location = 3) out vec3 cameraPos;
layout(location = 4) out vec4 fragPos;

#define BLEND_START		8		// m
#define BLEND_END		200		// m

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

layout(set = 3, binding = 0) uniform AmplitudeBufferObj {
	float amplitude;
	float choppy;
} abo;

layout(set = 4, binding = 0, rgba32f) uniform readonly image2D heightmap;
layout(set = 5, binding = 0, rgba32f) uniform readonly image2D normalmap;

void main() {

	// NOTE: also defined in fragment shader
	const vec3 perlinFrequency	= vec3(1.12, 0.59, 0.23);
	const vec3 perlinAmplitude	= vec3(0.35, 0.42, 0.57);

    mat4 model = objectBuffer.objects[gl_InstanceIndex].model;
    mat4 MVP =  camera.proj * camera.view * model;
	
	vec4 Displacement = imageLoad(heightmap, ivec2(inTexCoord));
	Displacement.y *= abo.amplitude;
	
	vec4 worldPos = model * vec4(inPosition.xyz + Displacement.xyz, 1.0f);
	
	fragPos.xyz = inPosition.xyz;
	fragPos.y =+ Displacement.y;
	fragPos.w = Displacement.w;
	
    gl_Position =  MVP * vec4(inPosition.xyz + Displacement.xyz, 1.0);
  //  gl_Position =  MVP * vec4(inPosition.xyz, 1.0f);
    texCoord = inTexCoord;
	
  //  fragColor = objectBuffer.objects[gl_InstanceIndex].color.xyz;
    fragColor = vec3(0.0, Displacement.y, 0.0);
	
	vec4 slope = imageLoad(normalmap, ivec2(inTexCoord));
	normal.xyz = normalize(vec3(
        - ( slope.x / (1.0f + abo.choppy * slope.z) ),
        1.0f,
        - ( slope.y / (1.0f + abo.choppy * slope.w) )
    ));
	normal.w = slope.w;
	//normal = vec4(0.0, 1.0, 0.0, 0.0);
	
	//normal = vec4(1.0, 1.0, 1.0, 1.0);
	
	cameraPos = camera.pos.xyz;
	
	vec3 viewDir = cameraPos - fragPos.xyz;
}