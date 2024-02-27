#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 cameraPos;
layout(location = 4) out vec2 texCoord;
layout(location = 5) out vec3 reflectedVector;
layout(location = 6) out vec3 refractedVector;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 pos;
} camera;

//push constants block

struct WaveData
{
    vec2 direction;
    vec2 origin;
    float frequency;
    float amplitude;
    float phase;
    float steepness;
};


//all object matrices
layout(std140, set = 1, binding = 0) buffer WaveBuffer{

	WaveData waves[];
} waveBuffer;

layout (set = 2, binding = 0) uniform ParameterUBO {
    float totalTime;
} ubo;

struct ObjectData
{
    mat4 model;
    vec4 color;
};

//all object matrices
layout(std140, set = 3, binding = 0) buffer ObjectBuffer{

	ObjectData objects[];
} objectBuffer;


void main() {
	mat4 model = objectBuffer.objects[gl_InstanceIndex].model;
    mat4 MVP =  camera.proj * camera.view * model;
 
   // mat3 normalMatrix = transpose(inverse(mat3(model)));
   
   
    fragColor = objectBuffer.objects[gl_InstanceIndex].color.xyz;
   
   
    //normal = normalize(normalMatrix * inNormal);
   
   float height = 0.0f;
   
   float dx = 0.0f;
   float dz = 0.0f;
   
   for (int i = 0; i < waveBuffer.waves.length(); ++i) {
   
        WaveData currentWave = waveBuffer.waves[i];
		
		vec2 direction = currentWave.direction;
		
		float xz = inPosition.x * direction.x + inPosition.z * direction.y;
		
		height += sin( currentWave.frequency * xz + ubo.totalTime * currentWave.phase ) * currentWave.amplitude ;
		
		dx += currentWave.frequency * currentWave.amplitude * direction.x * cos( currentWave.frequency * xz + ubo.totalTime * currentWave.phase);
		dz += currentWave.frequency * currentWave.amplitude * direction.y * cos( currentWave.frequency * xz + ubo.totalTime * currentWave.phase); 
	
        
        // Do something with the current object
        // For example, you can access its properties like currentObject.position, currentObject.rotation, etc.
   }
   
   vec3 Tangent = vec3(1.0, 0.0, dx);
   vec3 Bitangent = vec3(0.0, 1.0, dz);
   
   normal = normalize( cross(Tangent, Bitangent) );
	
	   gl_Position =  MVP * vec4(inPosition.x, height, inPosition.z, 1.0);
	//   gl_Position =  MVP * vec4(inPosition, 1.0);
	
	vec4 worldCoord = model * vec4(inPosition,1.0);
    fragPos = worldCoord.xyz / worldCoord.w;
    
    fragColor = objectBuffer.objects[gl_InstanceIndex].color.xyz;
    cameraPos = camera.pos.xyz;
	
	vec3 unitNormal = normalize(normal);
	vec3 viewVector = normalize(fragPos - cameraPos);
	reflectedVector = reflect(viewVector, unitNormal);
	refractedVector = refract(viewVector, unitNormal, 1.0 / 1.33);
}