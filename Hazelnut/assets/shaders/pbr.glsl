// PBR Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Norm;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out vec3 v_Normal;

void main()
{
    v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Norm;
	v_TexCoord = a_TexCoord;

    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color1;
layout(location = 1) out int color2;

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec3 v_Normal;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform vec3 lightDir;
uniform vec3 lightColor;

uniform vec3 viewPos;
uniform float u_UseNormalMap;

uniform int u_entityID;

const float PI = 3.14159265359;
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, v_TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(v_FragPos);
    vec3 Q2 = dFdy(v_FragPos);
    vec2 st1 = dFdx(v_TexCoord);
    vec2 st2 = dFdy(v_TexCoord);

    vec3 N = normalize(v_Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    //albedo textures are already in sRGB space
    vec3 albedo = pow(texture(albedoMap, v_TexCoord).rgb, vec3(2.2));
    float metallic = texture(metallicMap, v_TexCoord).r;
    float roughness = texture(roughnessMap, v_TexCoord).r;
    float ao = texture(aoMap, v_TexCoord).r;

    //color1 = vec4(vec3(albedo), 1.0);
    //return;


    vec3 N = normalize(v_Normal);
    if (u_UseNormalMap > 0.5)
        N = getNormalFromMap();

    vec3 V = normalize(viewPos - v_FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // energy conservation
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    // ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    color1 = vec4(color, 1.0);
    color2 = u_entityID;
}