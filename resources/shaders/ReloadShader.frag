#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragPos;
layout(location = 2) in vec3 position;

layout(location = 0) out vec4 outColor;

layout (set = 2, binding = 0) uniform ParameterUBO {
    float reloadTime;
} ubo;

void main() {

	if((position.x * position.x + position.y * position.y > 0.5 * 0.5) ||	(position.x * position.x + position.y * position.y < 0.3 * 0.3)) discard; 
	
	float dotProduct = dot(normalize(position.xy), vec2(0, 1) );
	
	if(ubo.reloadTime <= 0.5)
	{
		
		if(position.x < 0) discard;
		

		
		if( (ubo.reloadTime - 0.5) * 4 + 1 < dotProduct ) discard;
	
	}
	else if (ubo.reloadTime >= 0.5 && position.x < 0)
	{
		 if( (ubo.reloadTime - 1) * (-4) - 1 >= dotProduct) discard;
	}
	
		
    outColor = fragColor;
}
