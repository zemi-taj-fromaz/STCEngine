#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 cameraPos;
layout(location = 4) in vec2 texCoord;

struct PointLight
{
    vec4 position;
    vec4 ambientColor;
    vec4 diffColor;
    vec4 specColor;
    vec4 clq;
	
	int size;
};

struct FlashLight
{
    vec4 position;
    vec4 ambientColor;
    vec4 diffColor;
    vec4 specColor;
    vec4 clq;

    vec4 direction;
    float innerCutoff;
    float outerCutoff;
	
	int size;

};

struct GlobalLight
{
    vec4 ambientColor;
    vec4 diffColor;
    vec4 specColor;
    vec4 direction;
};

//all object matrices
layout(std140, set = 2, binding = 0) buffer PointLightBuffer{

	PointLight objects[];
} pointLights;

//all object matrices
layout(std140, set = 3, binding = 0) buffer FlashLightBuffer{

	FlashLight objects[];
} flashLights;

//all object matrices
layout(set = 4, binding = 0) uniform GlobalLightUniform{

	GlobalLight light;
} global;

layout(location = 0) out vec4 outColor;

vec3 CalcDirLight(GlobalLight light, vec3 normal, vec3 viewDir, vec3 fragColor)
{
    vec3 lightDir = normalize(-light.direction.xyz);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // combine results
    vec3 ambient  = light.ambientColor.xyz  * fragColor;
    vec3 diffuse  = light.diffColor.xyz  * diff * fragColor;
    vec3 specular = light.specColor.xyz * spec * fragColor;
    return (ambient + diffuse + specular);
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 fragColor)
{

	

    vec3 lightDir = normalize(light.position.xyz - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);


    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);

    float distance    = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.clq.x + light.clq.y * distance + 
  			     light.clq.z * (distance * distance));    

    vec3 ambient  = light.ambientColor.xyz  * fragColor;
    vec3 diffuse  = light.diffColor.xyz  * diff * fragColor;
    vec3 specular = light.specColor.xyz * spec * fragColor;
	
    ambient  *= attenuation;
    diffuse  *= attenuation;

	specular *= attenuation;

	
    return (ambient + diffuse + specular);
}


vec3 CalcFlashLight(FlashLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 fragColor)
{
    vec3 lightDir = normalize(light.position.xyz - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	
	float theta     = dot(lightDir, normalize(-light.direction.xyz));
	float epsilon   = light.innerCutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);    



    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);

    float distance    = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.clq.x + light.clq.y * distance + 
  			     light.clq.z * (distance * distance));    

    vec3 ambient  = light.ambientColor.xyz  * fragColor;
    vec3 diffuse  = light.diffColor.xyz  * diff * fragColor;
    vec3 specular = light.specColor.xyz * spec * fragColor;
	
    ambient  *= attenuation;
    diffuse  *= attenuation;
    diffuse *= intensity;
	specular *= attenuation;
	specular *= intensity;
	
    return (ambient + diffuse + specular);
}

void main() {	
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(cameraPos - fragPos);
	
	vec3 result = CalcDirLight(global.light, norm, viewDir, fragColor);

	int numPointLights = pointLights.objects[0].size;

    // Iterate through all objects
    for (int i = 0; i < numPointLights; ++i) 
	{
		result += CalcPointLight(pointLights.objects[i], norm, fragPos, viewDir, fragColor);    
	}
	
	int numFlashLights = flashLights.objects[0].size;
	
	for (int i = 0; i < numFlashLights; ++i) 
	{
		result += CalcFlashLight(flashLights.objects[i], norm, fragPos, viewDir, fragColor);    
	}
    outColor = vec4(result, 1.0f);
}