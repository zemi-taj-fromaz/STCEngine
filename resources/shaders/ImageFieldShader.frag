#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec4 normal;
layout (location = 2) in vec2 texCoord;
layout(location = 3) in vec3 cameraPos;
layout(location = 4) in vec4 fragPos;
layout(location = 5) in float Time;
layout(location = 6) in vec3 sunDir;

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


#define M_PI 3.14159265358979323846
#define ONE_OVER_PI (1.0 / M_PI)

// Water and Air indices of refraction
#define WATER_IOR 1.33477
#define AIR_IOR 1.0
#define IOR_AIR2WATER (AIR_IOR / WATER_IOR)

#define NORMAL_WORLD_UP vec3(0.0, 1.0, 0.0)

#define TERRAIN_HEIGHT 0.0f
vec4 kTerrainBoundPlane = vec4(NORMAL_WORLD_UP, TERRAIN_HEIGHT);

struct Ray
{
    vec3 org;
    vec3 dir;
};

/**
 * @brief Returns sky luminance at view direction
 * @param kSunDir Normalized vector to sun
 * @param kViewDir Normalized vector to eye
 */
vec3 SkyLuminance(const in vec3 kSunDir, const in vec3 kViewDir);

/**
 * @brief Finds an intersection point of ray with the terrain
 * @return Distance along the ray; positive if intersects, else negative
 */
float IntersectTerrain(const in Ray ray);

vec3 TerrainNormal(const in vec2 p);

vec3 TerrainColor(const in vec2 p);

/**
 * @brief Fresnel reflectance for unpolarized incident light
 *  holds for both air-incident and water-incident cases
 * @param theta_i Angle of incidence between surface normal and the direction
 *  of propagation
 * @param theta_t Angle of the transmitted (refracted) ray relative to the surface
 *  normal
 */
float FresnelFull(in float theta_i, in float theta_t)
{
    // Normally incident light
    if (theta_i <= 0.00001)
    {
        const float at0 = (WATER_IOR-1.0) / (WATER_IOR+1.0);
        return at0*at0;
    }

    const float t1 = sin(theta_i-theta_t) / sin(theta_i+theta_t);
    const float t2 = tan(theta_i-theta_t) / tan(theta_i+theta_t);
    return 0.5 * (t1*t1 + t2*t2);
}

vec3 Attenuate(const in float kDistance, const in float kDepth)
{
    const float kScale = 0.1;
    return exp(-surface.absorpCoef  * kScale * kDistance 
               -surface.scatterCoef * kScale * kDepth);
}

/**
 * @brief Refraction of air incident light
 * @param kIncident Direction of the incident light, unit vector
 * @param kNormal Surface normal, unit vector
 * @return Direction of the refracted ray
 */
vec3 RefractAirIncident(const in vec3 kIncident, const in vec3 kNormal)
{
    return refract(kIncident, kNormal, IOR_AIR2WATER);
}

/**
 * @param p_g Point on the terrain
 * @param kIncidentDir Incident unit vector
 * @return Outgoing radiance from the terrain to the refracted direction on
 *  the water surface.
 */
vec3 ComputeTerrainRadiance(
    const in vec3 p_g,
    const in vec3 kIncidentDir
)
{
    // TODO better TERRAIN COLOR
    return TerrainColor(p_g.xz) * dot(TerrainNormal(p_g.xz), -kIncidentDir);
}

vec3 ComputeWaterSurfaceColor(
    const in Ray ray,
    const in vec3 p_w,
    const in vec3 kNormal, float t)
{
    const vec3 kLightDir = normalize(sunDir);

    // Reflection of camera ray
    const vec3 kReflectDir = reflect(ray.dir, kNormal);
    const vec3 kSkyReflected = SkyLuminance(kLightDir, kReflectDir);

    // Diffuse atmospheric skylight
    vec3 L_a;
    {
        const float kNdL = max(dot(kNormal, kLightDir), 0.0);
       // L_a = surface.skyIntensity * kNdL * kSkyReflected;
        L_a =  kNdL * kSkyReflected;
    }

    // Direction to camera
    const vec3 kViewDir = vec3(-ray.dir);

    // Amount of light coming directly from the sun reflected to the camera
    vec3 L_s;
    {

    // Blinn-Phong - more accurate when compared to ocean sunset photos

        const vec3 kHalfWayDir = normalize(kLightDir + kViewDir);
      //  const float specular = surface.specularIntensity *
        const float specular = 1.0 *
                clamp(
                    pow(
                        max( dot(kNormal, kHalfWayDir), 0.0 ),
                    32.0)
                   // surface.specularHighlights)
                , 0.0, 1.0);
   

      //  L_s = surface.sunIntensity * specular * kSkyReflected;
        L_s =  specular * kSkyReflected;
    }

    const vec3 kRefractDir = RefractAirIncident(-kLightDir, kNormal);

    // Light just below the surface transmitted through into the air
    {
    vec3 L_u;
        // Downwelling irradiance just below the water surface
        const vec3 E_d0 = M_PI * L_a;

        // Constant diffuse radiance just below the surface from sun and sky
        const vec3 L_df0 = (0.33*surface.backscatterCoef) /
                            surface.absorpCoef * (E_d0 * ONE_OVER_PI);
       const Ray kRefractRay = Ray( p_w, kRefractDir );

        const float t_g = IntersectTerrain(kRefractRay);
        const vec3 p_g = kRefractRay.org + t_g * kRefractRay.dir;
        const vec3 L_g = ComputeTerrainRadiance(p_g, kRefractRay.dir);

        L_u =        Attenuate(t_g, 0.0)                   * L_g +
              (1.0 - Attenuate(t_g, abs(p_w.y - p_g.y) ) ) * L_df0;
    }

    // Fresnel reflectivity to the camera
    float F_r = FresnelFull( dot(kNormal, kViewDir), dot(-kNormal, kRefractDir) );

    return F_r*(L_s + L_a);// + (1.0-F_r)*L_u;
}

void main ()
{

    const Ray ray = Ray(cameraPos, normalize(fragPos.xyz - cameraPos));
    const vec3 kNormal = normalize( normal.xyz );

    const vec3 p_w = vec3(fragPos.x, fragPos.y + surface.height, fragPos.z);
    vec3 color = ComputeWaterSurfaceColor(ray, p_w, kNormal, Time);

    // TODO foam
    if (fragPos.w < 0.0)
        color = vec3(1.0);

    outColor = vec4(color, 1.0);

    // Tone mapping
   // outColor = vec4(1.0) - exp(-outColor * 2.0f);
}


void calculatePerezDistribution( in float t, out vec3 A, out vec3 B, out vec3 C, out vec3 D, out vec3 E )
{
	A = vec3(  0.1787 * t - 1.4630, -0.0193 * t - 0.2592, -0.0167 * t - 0.2608 );
	B = vec3( -0.3554 * t + 0.4275, -0.0665 * t + 0.0008, -0.0950 * t + 0.0092 );
	C = vec3( -0.0227 * t + 5.3251, -0.0004 * t + 0.2125, -0.0079 * t + 0.2102 );
	D = vec3(  0.1206 * t - 2.5771, -0.0641 * t - 0.8989, -0.0441 * t - 1.6537 );
	E = vec3( -0.0670 * t + 0.3703, -0.0033 * t + 0.0452, -0.0109 * t + 0.0529 );
}



// =============================================================================
// Terrain functions

float Fbm4Noise2D(in vec2 p);

float TerrainHeight(const in vec2 p)
{
    return TERRAIN_HEIGHT - 8.f * Fbm4Noise2D(p.yx * 0.02f);
}

vec3 TerrainNormal(const in vec2 p)
{
    // Approximate normal based on central differences method for height map
    const vec2 kEpsilon = vec2(0.0001, 0.0);

    return normalize(
        vec3(
            // x + offset
            TerrainHeight(p - kEpsilon.xy) - TerrainHeight(p + kEpsilon.xy), 
            10.0f * kEpsilon.x,
            // z + offset
            TerrainHeight(p - kEpsilon.yx) - TerrainHeight(p + kEpsilon.yx)
        )
    );
}

vec3 TerrainColor(const in vec2 p)
{
    //const vec3 kMate = vec3(0.964, 1.0, 0.824);
    float n = clamp(Fbm4Noise2D(p.yx * 0.02 * 2.), 0.6, 0.9);

    return n * surface.terrainColor;
}

float IntersectTerrain(const in Ray ray)
{
    return -( dot(ray.org, kTerrainBoundPlane.xyz) - kTerrainBoundPlane.w ) /
           dot(ray.dir, kTerrainBoundPlane.xyz);
}

// =============================================================================
// Noise functions

float hash1(in vec2 i)
{
    i = 50.0 * fract( i * ONE_OVER_PI );
    return fract( i.x * i.y * (i.x + i.y) );
}

float Noise2D(const in vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

#ifdef INTERPOLATION_CUBIC
    vec2 u = f*f * (3.0-2.0*f);
#else
    vec2 u = f*f*f * (f * (f*6.0-15.0) +10.0);
#endif
    float a = hash1(i + vec2(0,0) );
    float b = hash1(i + vec2(1,0) );
    float c = hash1(i + vec2(0,1) );
    float d = hash1(i + vec2(1,1) );

    return -1.0 + 2.0 * (a + 
                         (b - a) * u.x + 
                         (c - a) * u.y + 
                         (a - b - c + d) * u.x * u.y);
}

const mat2 MAT345 = mat2( 4./5., -3./5.,
                          3./5.,  4./5. );

float Fbm4Noise2D(in vec2 p)
{
    const float kFreq = 2.0;
    const float kGain = 0.5;
    float amplitude = 0.5;
    float value = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        value += amplitude * Noise2D(p);
        amplitude *= kGain;
        p = kFreq * MAT345 * p;
    }
    return value;
}

// =============================================================================
// Preetham Sky

float SaturateDot(const in vec3 v, const in vec3 u)
{
    return max( dot(v,u), 0.0f );
}

vec3 ComputePerezLuminanceYxy(const in float theta, const in float gamma)
{
    const float kBias = 1e-3f;
    return (1.f + surface.A * exp( surface.B / (cos(theta)+kBias) ) ) *
           (1.f + surface.C * exp( surface.D * gamma) +
            surface.E * cos(gamma) * cos(gamma) );
}

vec3 calculatePerezLuminanceYxy( in float theta, in float gamma, in vec3 A, in vec3 B, in vec3 C, in vec3 D, in vec3 E )
{
	return ( 1.0 + A * exp( B / cos( theta ) ) ) * ( 1.0 + C * exp( D * gamma ) + E * cos( gamma ) * cos( gamma ) );
}

vec3 calculateZenithLuminanceYxy( in float t, in float thetaS )
{
	float chi  	 	= ( 4.0 / 9.0 - t / 120.0 ) * ( M_PI - 2.0 * thetaS );
	float Yz   	 	= ( 4.0453 * t - 4.9710 ) * tan( chi ) - 0.2155 * t + 2.4192;

	float theta2 	= thetaS * thetaS;
    float theta3 	= theta2 * thetaS;
    float T 	 	= t;
    float T2 	 	= t * t;

	float xz =
      ( 0.00165 * theta3 - 0.00375 * theta2 + 0.00209 * thetaS + 0.0)     * T2 +
      (-0.02903 * theta3 + 0.06377 * theta2 - 0.03202 * thetaS + 0.00394) * T +
      ( 0.11693 * theta3 - 0.21196 * theta2 + 0.06052 * thetaS + 0.25886);

    float yz =
      ( 0.00275 * theta3 - 0.00610 * theta2 + 0.00317 * thetaS + 0.0)     * T2 +
      (-0.04214 * theta3 + 0.08970 * theta2 - 0.04153 * thetaS + 0.00516) * T +
      ( 0.15346 * theta3 - 0.26756 * theta2 + 0.06670 * thetaS + 0.26688);

	return vec3( Yz, xz, yz );
}

vec3 ComputeSkyLuminance(const in vec3 s, const in vec3 e, float t)
{
	
	vec3 A, B, C, D, E;
	calculatePerezDistribution( t, A, B, C, D, E );

	float thetaS = acos( SaturateDot( s, vec3(0,1,0) ) );
	float thetaE = acos( SaturateDot( e, vec3(0,1,0) ) );
	float gammaE = acos( SaturateDot( s, e )		   );

	vec3 Yz = calculateZenithLuminanceYxy( t, thetaS );

	vec3 fThetaGamma = calculatePerezLuminanceYxy( thetaE, gammaE, A, B, C, D, E );
	vec3 fZeroThetaS = calculatePerezLuminanceYxy( 0.0,    thetaS, A, B, C, D, E );

	vec3 Yp = Yz * ( fThetaGamma / fZeroThetaS );
	return Yp;
}

vec3 YxyToRGB(const in vec3 Yxy);

vec3 SkyLuminance(const in vec3 kSunDir, const in vec3 kViewDir)
{
    const float kSunDisk = smoothstep(0.997f, 1.f,
                                      SaturateDot(kViewDir, kSunDir) );

	float turbidity = 2.0;
    const vec3 kSkyLuminance = YxyToRGB( ComputeSkyLuminance(kSunDir, kViewDir, turbidity) );
    return kSkyLuminance * 0.05f + kSunDisk * kSkyLuminance;
}

// =============================================================================
// Althar. Preetham Sky. [online]. Shadertoy.com. 2015.
// https://www.shadertoy.com/view/llSSDR

vec3 YxyToXYZ( const in vec3 Yxy )
{
    const float Y = Yxy.r;
    const float x = Yxy.g;
    const float y = Yxy.b;

    const float X = x * ( Y / y );
    const float Z = ( 1.0 - x - y ) * ( Y / y );

    return vec3(X,Y,Z);
}

vec3 XYZToRGB( const in vec3 XYZ )
{
    // CIE/E
    const mat3 M = mat3
    (
         2.3706743, -0.9000405, -0.4706338,
        -0.5138850,  1.4253036,  0.0885814,
          0.0052982, -0.0146949,  1.0093968
    );

    return XYZ * M;
}


vec3 YxyToRGB( const in vec3 Yxy )
{
    const vec3 XYZ = YxyToXYZ( Yxy );
    const vec3 RGB = XYZToRGB( XYZ );
    return RGB;
}

// =============================================================================