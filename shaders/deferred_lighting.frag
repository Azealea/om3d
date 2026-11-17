#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo_rough;
layout(binding = 1) uniform sampler2D in_normal_metal;
layout(binding = 2) uniform sampler2D in_depth;

layout(binding = 4) uniform samplerCube in_envmap;
layout(binding = 5) uniform sampler2D brdf_lut;
layout(binding = 6) uniform sampler2DShadow in_shadow_map;

layout(binding = 0) uniform Data
{
    FrameData frame;
};

layout(binding = 1) buffer PointLights
{
    PointLight point_lights[];
};

void main()
{
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const float depth = texelFetch(in_depth, coord, 0).r;

    if(depth <= 0.0)
    {
        const vec3 sky_dir = normalize(unproject(in_uv, 0.001, frame.camera.inv_view_proj) - frame.camera.position);
        out_color = vec4(texture(in_envmap, sky_dir).rgb * frame.ibl_intensity, 1.0);
        return;
    }

    vec4 albedo_rough = texelFetch(in_albedo_rough, coord, 0);
    vec4 normal_metal = texelFetch(in_normal_metal, coord, 0);

    vec3 base_color = albedo_rough.rgb;
    float roughness = albedo_rough.a;
    vec3 normal = normalize(normal_metal.rgb * 2.0 - 1.0);
    float metal = normal_metal.a;

    vec3 position = unproject(in_uv, depth, frame.camera.inv_view_proj);

    vec3 view_dir = normalize(frame.camera.position - position);

    vec3 acc = eval_ibl(in_envmap, brdf_lut, normal, view_dir, base_color, metal, roughness) * frame.ibl_intensity;

    vec4 sun_clip = frame.sun_view_proj * vec4(position + frame.sun_dir * frame.sun_bias, 1.0);
    vec2 shadow_uv = sun_clip.xy * 0.5 + 0.5;
    vec2 shadow_texel = 1.0 / vec2(textureSize(in_shadow_map, 0));

    float shadow = 0.0;
    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            shadow += texture(in_shadow_map, vec3(shadow_uv + vec2(i, j) * shadow_texel, sun_clip.z));
        }
    }
    shadow /= 9.0;

    acc += shadow * frame.sun_color * eval_brdf(normal, view_dir, frame.sun_dir, base_color, metal, roughness);

    for(uint i = 0; i != frame.point_light_count; i++)
    {
        PointLight light = point_lights[i];
        vec3 to_light = (light.position - position);
        float dist = length(to_light);
        vec3 light_vec = to_light / dist;

        float att = attenuation(dist, light.radius);
        if(att <= 0.0f)
        {
            continue;
        }

        acc += eval_brdf(normal, view_dir, light_vec, base_color, metal, roughness) * att * light.color;
    }

    out_color = vec4(acc, 1.0);
}
