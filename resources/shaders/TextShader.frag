#version 460

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D text;
layout(set = 2, binding = 0) uniform ColorBufferObject
{
	vec3 textColor;
} cbo;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    outColor = vec4(cbo.textColor, 1.0) * sampled;
}