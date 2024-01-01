#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 pos;
} camera;

void main() {
    vec4 pos = vec4(inPosition, 1.0);
    gl_Position = camera.proj * camera.view * pos;
    fragColor = inColor;
    gl_PointSize = 4.0;
}
