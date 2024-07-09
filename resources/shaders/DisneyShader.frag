#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 wo;
layout(location = 4) in vec3 wi;

layout(location = 0) out vec4 outColor;

layout(set = 3, binding = 0) uniform DisneyShadingParams{
    float subsurface;
    float metallic;
    float specular;
    float specularTint;
    float roughness;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
} dsp;

const float PI = 3.14159265358979323846;

vec4 BRDF_Lambertian(vec3 wi, vec3 wo, vec3 normal)
{
	float color = dot(normalize(wi), normalize(normal));
	return vec4(color,color,color, 1.0);
}

vec3 BRDF_LambertianDiffuse(vec3 wi, vec3 wo, vec3 normal, vec3 fragColor)
{
	float cos_phi = dot(normalize(wi), normalize(normal)); // * (Light Color)
	return cos_phi * fragColor;
}

vec3 BRDF_BlinnPhong(vec3 wi, vec3 wo, vec3 normal, vec3 fragColor)
{
	float shininess = 32.0;
	vec3 H = (wi + wo) / 2.0;
	float cos_phi = dot(normalize(wi), normalize(normal)); // 
	float shine = pow( dot(normalize(H), normalize(normal)), shininess); // * (Light Color)
	return cos_phi*fragColor + vec3(cos_phi*shine);
}

float sqr(float x) { return x*x; }

vec3 mon2lin(vec3 x)
{
    return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}
float F_Schlick(float cos_phi_d)
{
    float m = clamp(1-cos_phi_d, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}
float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}
float smithG_GGX(float NdotV, float alphaG)
{
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}
float GTR1(float NdotH, float a)
{
    if (a >= 1) return 1/PI;
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return (a2-1) / (PI*log(a2)*t);
}


vec3 BRDF_Disney(vec3 l, vec3 v, vec3 normal, vec3 fragColor, vec3 X, vec3 Y)
{
	vec3 H = normalize(l + v);
	
	float cos_phi = dot(normalize(l), normalize(normal)); // 
	float cos_phi_d = dot(normalize(l), normalize(H));
	float cos_phi_l = dot(normalize(l), normalize(normal));
	float cos_phi_v = dot(normalize(v), normalize(normal));
	float cos_phi_h = dot(normalize(H), normalize(normal));
	
	if (cos_phi_v < 0 || cos_phi_l < 0) return vec3(0);
	
    vec3 Cdlin = mon2lin(fragColor);
    float Cdlum = .3*Cdlin[0] + .6*Cdlin[1]  + .1*Cdlin[2]; // luminance approx.

    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1); // normalize lum. to isolate hue+sat
    vec3 Cspec0 = mix(dsp.specular*.08*mix(vec3(1), Ctint, dsp.specularTint), Cdlin, dsp.metallic);
    vec3 Csheen = mix(vec3(1), Ctint, dsp.sheenTint);
	
	float FL = F_Schlick(cos_phi_l), FV = F_Schlick(cos_phi_v);
    float Fd90 = 0.5 + 2 * cos_phi_d*cos_phi_d * dsp.roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);
	
	float Fss90 = cos_phi_d*cos_phi_d*dsp.roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (cos_phi_l + cos_phi_v) - .5) + .5);

	float aspect = sqrt(1-dsp.anisotropic*.9);
    float ax = max(.001, sqr(dsp.roughness)/aspect);
    float ay = max(.001, sqr(dsp.roughness)*aspect);
    float Ds = GTR2_aniso(cos_phi_h, dot(H, X), dot(H, Y), ax, ay);
    float FH = F_Schlick(cos_phi_d);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs  = smithG_GGX_aniso(cos_phi_l, dot(l, X), dot(l, Y), ax, ay);
    Gs *= smithG_GGX_aniso(cos_phi_v, dot(l, X), dot(v, Y), ax, ay);

	vec3 Fsheen = FH * dsp.sheen * Csheen;
	
	// clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(cos_phi_h, mix(.1,.001,dsp.clearcoatGloss));
    float Fr = mix(.04, 1.0, FH);
    float Gr = smithG_GGX(cos_phi_l, .25) * smithG_GGX(cos_phi_v, .25);
	
	return ((1/PI) * mix(Fd, ss, dsp.subsurface)*Cdlin + Fsheen)
        * (1-dsp.metallic)
        + Gs*Fs*Ds + .25*dsp.clearcoat*Gr*Fr*Dr;
}


vec3 computeTangent(vec3 normal)
{
    vec3 up = vec3(0.0, 1.0, 0.0);
    if (abs(dot(normal, up)) > 0.99)
    {
        up = vec3(1.0, 0.0, 0.0);
    }
    
    vec3 tangent = normalize(cross(up, normal));
    return tangent;
}

vec3 computeBitangent(vec3 normal, vec3 tangent)
{
    vec3 bitangent = cross(normal, tangent);
    return bitangent;
}

void main() {
		
		//global light
//	vec3 wi = vec3(0.0, 0.0, -1.0);
	vec3 tangent = computeTangent(normalize(normal));
	vec3 bitangent = computeBitangent(normalize(normal), tangent);

    outColor = vec4( BRDF_Disney(wi, wo, normal, fragColor, tangent, bitangent), 1.0);
}

