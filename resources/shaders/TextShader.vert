#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 texCoord;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 pos;
} camera;


void main() {
    mat4 MVP =  camera.proj;
    gl_Position =  MVP * vec4(inPosition.xy, 0.0, 1.0);
    texCoord = inTexCoord;
}