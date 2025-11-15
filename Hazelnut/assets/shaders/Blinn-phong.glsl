// Blinn-phong Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Norm;
//layout(location = 2) in vec2 a_TexCoord;
//layout(location = 3) in vec4 a_Color;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

//out vec4 v_Color;
//out vec2 v_TexCoord;
out vec3 v_FragPos;
out vec3 v_Normal;

void main()
{
	v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
	v_Normal = mat3(transpose(inverse(u_Transform))) * a_Norm;

	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

in vec3 v_FragPos;
in vec3 v_Normal;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;

uniform int u_entityID;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 ambient, vec3 diffuse, vec3 specular, float shininess);

void main()
{
    vec3 norm = normalize(v_Normal);
    vec3 viewDir = normalize(viewPos - v_FragPos);

    // phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir, material.ambient, material.diffuse, material.specular, material.shininess);

    color = vec4(result, 1.0);

	color2 = u_entityID;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 ambient, vec3 diffuse, vec3 specular, float shininess)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    // Phong
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Blinn-Phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    // combine results
    vec3 _ambient = light.ambient * ambient;
    vec3 _diffuse = light.diffuse * diff * diffuse;
    vec3 _specular = light.specular * spec * specular;
    return (_ambient + _diffuse + _specular);
}