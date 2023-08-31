#version 400 core
#define COOK_BLINN
#define COOK
#define USE_ALBEDO_MAP
#define USE_NORMAL_MAP
// #define USE_ROUGHNESS_MAP

in vec2 v_texcoord; // texture coords
in vec<3> v_normal;   // normal
in vec<3> v_binormal; // binormal (for TBN basis calc)
in vec<3> v_pos;      // pixel view space position

out vec4 color;


uniform mat<4, 4> world_matrix;  // object's world position
uniform mat<4, 4> view_matrix;   // view (camera) transform
uniform mat<4, 4> proj_matrix;   // projection matrix
uniform mat<3, 3> normal_matrix; // normal transformation matrix ( transpose(inverse(W * V)) )


uniform vec4 material; // x - metallic, y - roughness, w - "rim" lighting
uniform vec4 albedo;   // constant albedo color, used when textures are off


uniform sampler2D tex;     // base texture (albedo)
uniform sampler2D norm;    // normal map
uniform sampler2D spec;    // "factors" texture (G channel used as roughness)
uniform sampler2D iblbrdf; // IBL BRDF normalization precalculated tex
uniform samplerCube envd;  // prefiltered env cubemap

#define PI 3.1415926


// constant light position, only one light source for testing (treated as point light)
const vec4 light_pos = vec4(0, 1e6, 1e6, 1);


// handy value clamping to 0 - 1 range
float saturate(in float value)
{
    return clamp(value, 0.0, 1.0);
}


// phong (lambertian) diffuse term
float phong_diffuse()
{
    return (1.0 / PI);
}


// compute fresnel specular factor for given base specular and product
// product could be NdV or VdH depending on used technique
vec<3> fresnel_factor(in vec<3> f0, in float product)
{
    return mix(f0, vec<3>(1.0), pow(1.01 - product, 5.0));
}


// following functions are copies of UE4
// for computing cook-torrance specular lighting terms

float D_blinn(in float roughness, in float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float n = 2.0 / m2 - 2.0;
    return (n + 2.0) / (2.0 * PI) * pow(NdH, n);
}

float D_beckmann(in float roughness, in float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float NdH2 = NdH * NdH;
    return exp((NdH2 - 1.0) / (m2 * NdH2)) / (PI * m2 * NdH2 * NdH2);
}

float D_GGX(in float roughness, in float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float d = (NdH * m2 - NdH) * NdH + 1.0;
    return m2 / (PI * d * d);
}

float G_schlick(in float roughness, in float NdV, in float NdL)
{
    float k = roughness * roughness * 0.5;
    float V = NdV * (1.0 - k) + k;
    float L = NdL * (1.0 - k) + k;
    return 0.25 / (V * L);
}


// simple phong specular calculation with normalization
vec<3> phong_specular(in vec<3> V, in vec<3> L, in vec<3> N, in vec<3> specular, in float roughness)
{
    vec<3> R = reflect(-L, N);
    float spec = max(0.0, dot(V, R));

    float k = 1.999 / (roughness * roughness);

    return min(1.0, 3.0 * 0.0398 * k) * pow(spec, min(10000.0, k)) * specular;
}

// simple blinn specular calculation with normalization
vec<3> blinn_specular(in float NdH, in vec<3> specular, in float roughness)
{
    float k = 1.999 / (roughness * roughness);

    return min(1.0, 3.0 * 0.0398 * k) * pow(NdH, min(10000.0, k)) * specular;
}

// cook-torrance specular calculation
vec<3> cooktorrance_specular(in float NdL, in float NdV, in float NdH, in vec<3> specular, in float roughness)
{
#ifdef COOK_BLINN
    float D = D_blinn(roughness, NdH);
#endif

#ifdef COOK_BECKMANN
    float D = D_beckmann(roughness, NdH);
#endif

#ifdef COOK_GGX
    float D = D_GGX(roughness, NdH);
#endif

    float G = G_schlick(roughness, NdV, NdL);

    float rim = mix(1.0 - roughness * material.w * 0.9, 1.0, NdV);

    return (1.0 / rim) * specular * G * D;
}


void main() {
    // point light direction to point in view space
    vec<3> local_light_pos = (view_matrix * (/*world_matrix */ light_pos)).xyz;

    // light attenuation
    float A = 1.0;// / dot(local_light_pos - v_pos, local_light_pos - v_pos);

    // L, V, H vectors
    vec<3> L = normalize(local_light_pos - v_pos);
    vec<3> V = normalize(-v_pos);
    vec<3> H = normalize(L + V);
    vec<3> nn = normalize(v_normal);

    vec<3> nb = normalize(v_binormal);
    mat<3, 3> tbn = mat<3, 3>(nb, cross(nn, nb), nn);


    vec2 texcoord = v_texcoord;


    // normal map
#ifdef USE_NORMAL_MAP
    // tbn basis
    vec<3> N = tbn * (texture(norm, texcoord).xyz * 2.0 - 1.0);
#else
    vec<3> N = nn;
#endif

    // albedo/specular base
#ifdef USE_ALBEDO_MAP
    vec<3> base = texture(tex, texcoord).xyz;
#else
    vec<3> base = albedo.xyz;
#endif

    // roughness
#ifdef USE_ROUGHNESS_MAP
    float roughness = texture(spec, texcoord).y * material.y;
#else
    float roughness = material.y;
#endif

    // material params
    float metallic = material.x;

    // mix between metal and non-metal material, for non-metal
    // constant base specular factor of 0.04 grey is used
    vec<3> specular = mix(vec<3>(0.04), base, metallic);

    // TODO: put env map back
    // diffuse IBL term
    //    I know that my IBL cubemap has diffuse pre-integrated value in 10th MIP level
    //    actually level selection should be tweakable or from separate diffuse cubemap
    mat<3, 3> tnrm = transpose(normal_matrix);
    vec<3> envdiff = textureLod(envd, tnrm * N, 10).xyz;
    // vec<3> envdiff = texture(envd, tnrm * N).xyz;

    // specular IBL term
    //    11 magic number is total MIP levels in cubemap, this is simplest way for picking
    //    MIP level from roughness value (but it's not correct, however it looks fine)
    vec<3> refl = tnrm * reflect(-V, N);
    vec<3> envspec = textureLod(envd, refl, max(roughness * 11.0, textureQueryLod(envd, refl).y)).xyz;
    // vec<3> envspec = texture(envd, nn).xyz;


    // compute material reflectance

    float NdL = max(0.0, dot(N, L));
    float NdV = max(0.001, dot(N, V));
    float NdH = max(0.001, dot(N, H));
    float HdV = max(0.001, dot(H, V));
    float LdV = max(0.001, dot(L, V));

    // fresnel term is common for any, except phong
    // so it will be calcuated inside ifdefs


#ifdef PHONG
    // specular reflectance with PHONG
    vec<3> specfresnel = fresnel_factor(specular, NdV);
    vec<3> specref = phong_specular(V, L, N, specfresnel, roughness);
#endif

#ifdef BLINN
    // specular reflectance with BLINN
    vec<3> specfresnel = fresnel_factor(specular, HdV);
    vec<3> specref = blinn_specular(NdH, specfresnel, roughness);
#endif

#ifdef COOK
    // specular reflectance with COOK-TORRANCE
    vec<3> specfresnel = fresnel_factor(specular, HdV);
    vec<3> specref = cooktorrance_specular(NdL, NdV, NdH, specfresnel, roughness);
#endif

    specref *= vec<3>(NdL);

    // diffuse is common for any model
    vec<3> diffref = (vec<3>(1.0) - specfresnel) * phong_diffuse() * NdL;


    // compute lighting
    vec<3> reflected_light = vec<3>(0);
    vec<3> diffuse_light = vec<3>(0); // initial value == constant ambient light

    // point light
    vec<3> light_color = vec<3>(1.0) * A;
    reflected_light += specref * light_color;
    diffuse_light += diffref * light_color;

    // IBL lighting
    vec2 brdf = texture(iblbrdf, vec2(roughness, NdV)).xy;
    vec<3> iblspec = min(vec<3>(0.99), fresnel_factor(specular, NdV) * brdf.x + brdf.y);
    reflected_light += iblspec * envspec;
    diffuse_light += envdiff * (1.0 / PI);

    // final result
    vec<3> result = diffuse_light * mix(base, vec<3>(0.0), metallic) + reflected_light;


    color = vec4(texture(envd,v_normal).xyz, 1.0);// + vec4(texture(tex, texcoord)) * 0.1;
    // color = vec4(texture(tex, texcoord));
}
