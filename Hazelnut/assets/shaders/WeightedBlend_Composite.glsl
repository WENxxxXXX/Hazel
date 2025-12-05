// WeightedBlend_Composite Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
//layout(location = 1) in vec3 a_Norm;
layout(location = 1) in vec2 a_TexCoord;
//layout(location = 3) in vec4 a_Color;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 frag;

layout (binding = 0) uniform sampler2D u_Accum;
layout (binding = 1) uniform sampler2D u_Reveal;

const float EPSILON = 0.00001;

in vec2 v_TexCoord;

bool isApproximatelyEqual(float a, float b)
{
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

void main()
{
    vec4 accumulation = texture(u_Accum, v_TexCoord);
    float revealage = texture(u_Reveal, v_TexCoord).r;

    if (isApproximatelyEqual(revealage, 1.0))
        discard;

    if (isinf(max3(abs(accumulation.rgb))))
        accumulation.rgb = vec3(accumulation.a);

    vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);

    frag = vec4(average_color, 1.0 - revealage);
}