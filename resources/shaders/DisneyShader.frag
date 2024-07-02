#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 wo;

layout(location = 0) out vec4 outColor;

vec4 BRDF( vec3 wi, vec3 wo, vec3 normal)
{
	
	float color = dot(normalize(wi), normalize(normal));
	return vec4(color,color,color, 1.0);
}

void main() {
		
	//global light
	vec3 wi = vec3(1.0, 1.0, 1.0);

    outColor = BRDF(wi, wo, normal);
}

