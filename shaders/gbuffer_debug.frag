#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo_rough;
layout(binding = 1) uniform sampler2D in_normal_metal;
layout(binding = 2) uniform sampler2D in_depth;

uniform uint mode = 0;

void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec4 albedo_rough = texelFetch(in_albedo_rough, coord, 0);
    vec4 normal_metal = texelFetch(in_normal_metal, coord, 0);

    if(mode == 0)
    {
        float depth = texelFetch(in_depth, coord, 0).r;
        depth = pow(depth, 0.35);
        out_color = vec4(depth, depth, depth, 1.0);
    }
    else if(mode == 1)
    {
        out_color = vec4(albedo_rough.rgb, 1.0);
    }
    else if(mode == 2)
    {
        out_color = vec4(normal_metal.rgb, 1.0);
    }
    else if(mode == 3)
    {
        out_color = vec4(vec3(albedo_rough.a), 1.0);
    }
    else
    {
        out_color = vec4(vec3(normal_metal.a), 1.0);
    }
}
