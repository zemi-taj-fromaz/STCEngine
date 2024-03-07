#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 cameraPos;
layout(location = 4) in vec2 texCoord;
layout(location = 5) in vec3 reflectedVector;
layout(location = 6) in vec3 refractedVector;

layout(location = 0) out vec4 outColor;

struct GlobalLight
{
    vec4 ambientColor;
    vec4 diffColor;
    vec4 specColor;
    vec4 direction;
};


//all object matrices
layout(set = 4, binding = 0) uniform GlobalLightUniform{

	GlobalLight light;
} global;


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

void main() {

	vec3 norm = normalize(normal);
	
	vec3 viewDir = normalize(cameraPos - fragPos);
	
	vec3 result = CalcDirLight(global.light, norm, viewDir, fragColor);
	
	//vec4 reflectedColor = texture(cubeSampler, reflectedVector);
	
    //outColor = vec4(result, 1.0f);
	//outColor = mix(outColor, reflectedColor, 0.8);

    outColor = vec4(fragColor,1.0f);
}