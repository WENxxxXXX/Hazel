// LinkedListOIT_Build Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Norm;
//layout(location = 2) in vec2 a_TexCoord;
//layout(location = 3) in vec4 a_Color;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

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

//必须显式开启，因为本着色器涉及原子计数器和图像存储，可能不会隐式开启提前深度测试；
//而如果不开启提前深度测试，则某些被不透明物体挡住的半透明片元会被写入到链表中，导致混合错误。
layout(early_fragment_tests) in;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float alpha;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 v_FragPos;
in vec3 v_Normal;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;

uniform int u_entityID;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 ambient, vec3 diffuse, vec3 specular, float shininess);

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform writeonly uimageBuffer list_buffer;
layout (binding = 0, offset = 0) uniform atomic_uint list_counter;

void main()
{
    uint index;
    uint old_head;
    uvec4 item;

    //得到旧值，即第一个执行片元为1，第二个为2...
    //相当于一个新的索引
    index = atomicCounterIncrement(list_counter);
    //链表的头部现在指向新索引，新索引的节点将指向old_head，头插
    old_head = imageAtomicExchange(head_pointer_image, ivec2(gl_FragCoord.xy), uint(index));

    vec3 viewDir = normalize(viewPos - v_FragPos);
    vec3 norm = normalize(dot(v_Normal, viewDir) * v_Normal);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, material.ambient, material.diffuse, material.specular, material.shininess);

    item.x = old_head;
    item.y = packUnorm4x8(vec4(result, material.alpha));
    item.z = floatBitsToUint(gl_FragCoord.z);
    item.w = u_entityID;
    imageStore(list_buffer, int(index), item);
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