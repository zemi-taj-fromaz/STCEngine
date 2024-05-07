#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec4 normal;
layout (location = 2) in vec2 texCoord;
layout(location = 3) in vec3 cameraPos;
layout(location = 4) in vec4 fragPos;

layout(set = 2, binding = 0) uniform WaterSurfaceUBO
{
    vec3  camPos;
    float height;
    vec3 absorpCoef;
    vec3 scatterCoef;
    vec3 backscatterCoef;
    // ------------- Terrain
    vec3 terrainColor;
    // ------------ Sky
    float skyIntensity;
    float specularIntensity;
    float specularHighlights;
    vec3  sunColor;
    float sunIntensity;
    // ------------- Preetham Sky
    vec3  sunDir;
    float turbidity;
    vec3 A;
    vec3 B;
    vec3 C;
    vec3 D;
    vec3 E;
    vec3 ZenithLum;
    vec3 ZeroThetaSun;
} surface;

layout(location = 0) out vec4 outColor;

// =============================================================================
#define ONE_OVER_4PI	0.0795774715459476
void main() {

	const vec3 sunColor			= vec3(1.0, 1.0, 0.47);
	const vec3 perlinFrequency	= vec3(1.12, 0.59, 0.23);
	const vec3 perlinGradient	= vec3(0.014, 0.016, 0.022);
	const vec3 sundir			= vec3(0.603, 0.240, -0.761);
	const vec3 oceanColor 		= vec3(0.000, 0.0103, 0.0331);

	vec3 vdir = cameraPos - fragPos.xyz;
	vec3 n = normalize(normal.xyz);
	vec3 v = normalize(vdir);
	vec3 l = reflect(-v, n);

	float F0 = 0.020018673;
	float F = F0 + (1.0 - F0) * pow(1.0 - dot(n, l), 5.0);

	//vec3 refl = texture(envmap, l).rgb;

	// tweaked from ARM/Mali's sample
	float turbulence = max(1.6 - normal.w, 0.0);
	float color_mod = 1.0 + 3.0 * smoothstep(1.2, 1.8, turbulence);

	//color_mod = mix(1.0, color_mod, factor);

	// some additional specular (Ward model)
	const float rho = 0.3;
	const float ax = 0.2;
	const float ay = 0.1;

	vec3 h = sundir + v;
	vec3 x = cross(sundir, n);
	vec3 y = cross(x, n);

	float mult = (ONE_OVER_4PI * rho / (ax * ay * sqrt(max(1e-5, dot(sundir, n) * dot(v, n)))));
	float hdotx = dot(h, x) / ax;
	float hdoty = dot(h, y) / ay;
	float hdotn = dot(h, n);

	float spec = mult * exp(-((hdotx * hdotx) + (hdoty * hdoty)) / (hdotn * hdotn));

	outColor = vec4(mix(oceanColor, oceanColor * color_mod, F) + sunColor * spec, 1.0);
	//outColor = vec4(fragColor, 1.0);
	
}