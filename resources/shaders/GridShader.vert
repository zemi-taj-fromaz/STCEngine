#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out float near; //0.01
layout(location = 1) out float far; //100
layout(location = 2) out vec3 nearPoint;
layout(location = 3) out vec3 farPoint;
layout(location = 4) out mat4 fragView;
layout(location = 8) out mat4 fragProj;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 pos;
} camera;

vec3 UnprojectPoint(float x, float y, float z, mat4 viewInv, mat4 projInv) {

    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    mat4 viewInv = inverse(camera.view);
    mat4 projInv = inverse(camera.proj);
    vec3 p = inPosition;
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, viewInv, projInv).xyz; // unprojecting on the near plane
    farPoint = UnprojectPoint(p.x, p.y, 1.0, viewInv, projInv).xyz; // unprojecting on the far plane
    gl_Position = vec4(p, 1.0); // using directly the clipped coordinates
    near = 0.01;
    far = 100;
    fragView = camera.view;
    fragProj = camera.proj;
}