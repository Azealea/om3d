#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_normal_metal;
layout(binding = 1) uniform sampler2D in_depth;

uniform uint mode = 0;

void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);

    if(mode == 0)
    {
        float depth = texelFetch(in_depth, coord, 0).r;
        depth = pow(depth, 0.35);
        out_color = vec4(depth, depth, depth, 1.0);
    }
    else
    {
        out_color = vec4(texelFetch(in_normal_metal, coord, 0).rgb, 1.0);
    }
}
