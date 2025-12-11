// LinkedListOIT_Resolve Shader

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

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;

layout(binding = 0) uniform sampler2D backgroundTexture;

in vec2 v_TexCoord;

layout (location = 0) out vec4 color;
layout(location = 1) out int color2;

#define MAX_FRAGMENTS 128

uvec4 fragment_list[MAX_FRAGMENTS];

void main()
{
    uint current_index;
    uint fragment_count = 0;

    current_index = imageLoad(head_pointer_image, ivec2(gl_FragCoord).xy).x;
    
    //0代表链表结束（应该用0xFFFFFFFF代表链表结束，影响不大）
    while (current_index != 0 && fragment_count < MAX_FRAGMENTS)
    {
        uvec4 fragment = imageLoad(list_buffer, int(current_index));
        fragment_list[fragment_count] = fragment;
        current_index = fragment.x;
        fragment_count++;
    }

    uint i, j;
    if (fragment_count > 1)
    {
        for (i = 0; i < fragment_count - 1; i++)
        {
            for (j = 0; j < fragment_count; j++)
            {
                uvec4 fragment1 = fragment_list[i];
                uvec4 fragment2 = fragment_list[j];

                float depth1 = uintBitsToFloat(fragment1.z);
                float depth2 = uintBitsToFloat(fragment2.z);

                if (depth1 < depth2)
                {
                    fragment_list[i] = fragment2;
                    fragment_list[j] = fragment1;
                }
            }
        }

        color2 = int(fragment_list[fragment_count - 1].w);
    }
    else
    {
        color2 = -1;
    }

    vec4 final_color = texture(backgroundTexture, v_TexCoord);
    for (i = 0; i < fragment_count; i++)
    {
        vec4 c = unpackUnorm4x8(fragment_list[i].y);

        final_color = mix(final_color, c, c.a);
    }

    color = final_color;
}