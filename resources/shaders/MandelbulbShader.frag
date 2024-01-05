#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 position;
layout(location = 3) in vec4 cameraPos;
layout(location = 4) in vec2 texCoord;

layout (set = 3, binding = 0) uniform ParameterUBO {
    float totalTime;
} ubo;


#define BOUNDING_RADIUS 1.1

#define COLOR1 vec3(1.0, 0.3, 0.0)
#define COLOR2 vec3(0.0, 0.7, 1.0)
#define BACKGROUND vec3(0.9, 0.8, 0.8)

#define ir3 0.57735

float mandelbulb(vec3 pos){
    vec3 w = pos;
    float dr = 1.0,r;
    vec3 p,p2,p4;
    float k1,k2,k3,k4,k5;

    for (int i = 0; i < 10; i++){
        r = dot(w, w);
        if (r > 4.0) break;
        dr =  pow(r, 3.5)*8.0*dr + 1.0;

        p = w;
        p2 = w * w;
        p4 = p2 * p2;

        k3 = p2.x + p2.z;
        k2 = inversesqrt( k3*k3*k3*k3*k3*k3*k3 );
        k1 = dot(p4, vec3(1)) - 6.0 * dot(p2, vec3(p2.y, p2.z, -p2.x / 3.0));
        k4 = dot(p2, vec3(1, -1, 1));
        k5 = 8.0*p.y*k4*k1*k2;

        w = pos + vec3(8.0*k5*p.x*p.z*(p2.x-p2.z)*(p4.x-6.0*p2.x*p2.z+p4.z),
                       -16.0*p2.y*k3*k4*k4 + k1*k1,
                       -k5*(p4.x*p4.x - 28.0*p4.x*p2.x*p2.z + 
                            70.0*p4.x*p4.z - 28.0*p2.x*p2.z*p4.z + p4.z*p4.z));
    }
    return log(r)*sqrt(r)/dr;
}

float dist(vec3 p) {
    return 0.385*mandelbulb(p);
}

bool bounding(in vec3 ro, in vec3 rd){
    float b = dot(rd,ro);
    return dot(ro,ro) - b*b < BOUNDING_RADIUS * BOUNDING_RADIUS;
}

vec2 march(vec3 ro, vec3 rd){
    if (bounding(ro, rd)){
        float t = 0.72, d;
        for (int i = 0; i < 96; i++){
            d = dist(ro + rd * t);
            t += d;

            if (d < 0.002) return vec2(t, d);
            if (d > 0.4) return vec2(-1.0);
        }
    }

    return vec2(-1.0);
}

vec3 normall(vec3 p){
    const float eps = 0.005;
    return normalize(vec3(dist(p+vec3(eps,0,0))-dist(p-vec3(eps,0,0)),
                          dist(p+vec3(0,eps,0))-dist(p-vec3(0,eps,0)),
                          dist(p+vec3(0,0,eps))-dist(p-vec3(0,0,eps))));
}



layout(location = 0) out vec4 outColor;

void main() {

	float theta = ubo.totalTime * 0.2;
    mat2 rot = mat2(+cos(theta), -sin(theta),
                    +sin(theta), +cos(theta));
    mat2 rrot = mat2(+cos(theta), +sin(theta),
                     -sin(theta), +cos(theta));
    vec2 rxz = vec2(0.0, -1.8) * rot;
    vec3 origin = vec3(rxz.x, sin(theta*1.61)*0.1, rxz.y);

	vec2 uv = texCoord;

    vec3 ray_direction =  normalize(vec3(uv, 1.1));
	ray_direction.xz *=rot;
	
	vec2 res = march(origin, ray_direction);

	
	//vec3 color = normalize(vec3(sin(theta) + 1.0 / AO_MAGIC, cos(phi) + 1.0 / AO_MAGIC, sin(phi + theta) + 1.0 / AO_MAGIC));
	//vec3 color = normalize(vec3(1.0,0.0,0.0));
	//outColor.rgb = ray_march(origin, ray_direction, color);
    //outColor.a = 1.0;
	
	if (res.x > 0.0){
        vec3 end = origin + ray_direction * res.x;

        vec3 norm = normall(end-ray_direction*0.001);

        float ao = clamp((dist(end + norm * 0.02) - res.y) / 0.02, 0.0, 1.0);
       // norm.xz *= rrot;

        float m = clamp(dot(end, end), 0.0, BOUNDING_RADIUS) / BOUNDING_RADIUS;
        vec3 col = mix(COLOR1, COLOR2, m*m*m);
		
        float d = max(dot(norm, vec3(-ir3)), 0.0);
        vec3 light = col * ao + 0.2 * d + 0.4 * d*d*d*d*d*d*d*d;

        outColor = vec4(light, 1.0);
    } else {
        outColor = vec4(BACKGROUND - length(uv) / 4.0, 1.0);
	discard;
    }

    //outColor = vec4(color + sceneData.ambientColor.xyz, 1.0f);
}