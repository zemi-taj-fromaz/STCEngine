#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 cameraPos;
layout(location = 4) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
  
float DistributionGGX (vec3 N, vec3 H, float roughness){
    float a2    = roughness * roughness * roughness * roughness;
    float NdotH = max (dot (N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX (float NdotV, float roughness){
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith (vec3 N, vec3 V, vec3 L, float roughness){
    return GeometrySchlickGGX (max (dot (N, L), 0.0), roughness) * 
           GeometrySchlickGGX (max (dot (N, V), 0.0), roughness);
}

vec3 FresnelSchlick (float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow (1.0 - cosTheta, 5.0);
}

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

layout(set = 5, binding = 0) uniform sampler2D texAlbedo;
layout(set = 6, binding = 0) uniform sampler2D texNormals;
layout(set = 7, binding = 0) uniform sampler2D texMetallic;
layout(set = 8, binding = 0) uniform sampler2D texRoughness;

void main()
{		
    //vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - fragPos);

	vec3 albedo = texture(texAlbedo,texCoord).xyz;
	float metallic = texture(texMetallic, texCoord).x;
	vec3 N = texture(texNormals, texCoord).xyz;
	float roughness = texture(texRoughness, texCoord).x;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo , metallic);
    // reflectance equation
    vec3 Lo = vec3(0.0);
	
	int numPointLights = pointLights.objects[0].size;
	int numFlashLights = flashLights.objects[0].size;


    // Iterate through all objects
  //  for (int i = 0; i < numPointLights; ++i) 
//	{
//		result += CalcPointLight(pointLights.objects[i], N, fragPos, viewDir, fragColor);    
//	}
	
    for(int i = 0; i < numPointLights; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(pointLights.objects[i].position.xyz - fragPos);
        vec3 H = normalize(V + L);
        float distance    = length(pointLights.objects[i].position.xyz - fragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = pointLights.objects[i].diffColor.xyz * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }
	
	for(int i = 0; i < numFlashLights; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(flashLights.objects[i].position.xyz - fragPos);
        vec3 H = normalize(V + L);
        float distance    = length(flashLights.objects[i].position.xyz - fragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = flashLights.objects[i].diffColor.xyz * attenuation;  

		float theta     = dot(L, normalize(-flashLights.objects[i].direction.xyz));
		float epsilon   = flashLights.objects[i].innerCutoff - flashLights.objects[i].outerCutoff;
		float intensity = clamp((theta - flashLights.objects[i].outerCutoff) / epsilon, 0.0, 1.0);   		
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * intensity; 
    }
  
    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    outColor = vec4(color, 1.0);
}  