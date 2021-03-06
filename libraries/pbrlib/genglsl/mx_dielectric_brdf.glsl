#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"

void mx_dielectric_brdf_reflection(vec3 L, vec3 V, float weight, vec3 tint, float ior, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = base;
        return;
    }

    vec3 Y = normalize(cross(N, X));

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);
    float VdotH = dot(V, H);

    float avgRoughness = mx_average_roughness(roughness);
    float F0 = mx_ior_to_f0(ior);

    float D = mx_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    float F = mx_fresnel_schlick(VdotH, F0);
    float G = mx_ggx_smith_G(NdotL, NdotV, avgRoughness);

    float comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    float dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, F0, 1.0) * comp;

    // Note: NdotL is cancelled out
    result = D * F * G * comp * tint * weight / (4 * NdotV) // Top layer reflection
           + base * (1.0 - dirAlbedo * weight);             // Base layer reflection attenuated by top layer
}

void mx_dielectric_brdf_transmission(vec3 V, float weight, vec3 tint, float ior, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    // Dielectric BRDF has no transmission but we must
    // attenuate the base layer transmission by the
    // inverse of top layer reflectance.

    float avgRoughness = mx_average_roughness(roughness);
    float F0 = mx_ior_to_f0(ior);

    // Abs here to allow transparency through backfaces
    float NdotV = abs(dot(N, V));
    float F = mx_fresnel_schlick(NdotV, F0);

    float comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    float dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, F0, 1.0) * comp;

    result = base * (1.0 - dirAlbedo * weight); // Base layer transmission attenuated by top layer
}

void mx_dielectric_brdf_indirect(vec3 V, float weight, vec3 tint, float ior, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float avgRoughness = mx_average_roughness(roughness);
    float F0 = mx_ior_to_f0(ior);

    vec3 Li = mx_environment_radiance(N, V, X, roughness, vec3(F0), vec3(1.0), distribution);

    float NdotV = dot(N, V);
    float F = mx_fresnel_schlick(NdotV, F0);

    float comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    float dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, F0, 1.0) * comp;

    result = Li * tint * comp * weight          // Top layer reflection
           + base * (1.0 - dirAlbedo * weight); // Base layer reflection attenuated by top layer
}
