#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_albedo_rough;
layout(location = 1) out vec4 out_normal_metal;
layout(location = 2) out vec4 out_emissive;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;
layout(binding = 2) uniform sampler2D in_metal_rough;
layout(binding = 3) uniform sampler2D in_emissive;

uniform vec3 base_color_factor;
uniform vec2 metal_rough_factor;
uniform float alpha_cutoff;
uniform vec3 emissive_factor;

void main()
{
    vec4 albedo_tex = texture(in_texture, in_uv);

    if(albedo_tex.a <= alpha_cutoff)
    {
        discard;
    }

    vec3 normal_map = unpack_normal_map(texture(in_normal_texture, in_uv).xy);
    vec3 normal = normalize(normal_map.x * in_tangent +
                            normal_map.y * in_bitangent +
                            normal_map.z * in_normal);

    vec3 base_color = in_color.rgb * albedo_tex.rgb * base_color_factor;

    vec4 metal_rough_tex = texture(in_metal_rough, in_uv);
    const float roughness = metal_rough_tex.g * metal_rough_factor.y; // as per glTF spec
    const float metal = metal_rough_tex.b * metal_rough_factor.x; // as per glTF spec

    out_albedo_rough = vec4(base_color, roughness);
    out_normal_metal = vec4(normal * 0.5 + 0.5, metal);
    out_emissive = vec4(texture(in_emissive, in_uv).rgb * emissive_factor, 1.0);
}
