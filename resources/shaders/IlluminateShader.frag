#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 cameraPos;
layout(location = 4) in vec2 texCoord;

layout(set = 1, binding = 0) uniform  SceneData{
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	vec4 sunPosition;
} sceneData;


layout(location = 0) out vec4 outColor;

void main() {	

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(sceneData.sunPosition.xyz - fragPos);


	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * sceneData.sunlightColor.xyz;

	float specularStrength = 0.5;
	vec3 viewDir = normalize(cameraPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * sceneData.sunlightColor.xyz;  

	vec3 result = (sceneData.ambientColor.xyz + diffuse) * fragColor;

    outColor = vec4(result, 1.0f);
}