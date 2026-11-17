#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_albedo_rough;
layout(binding = 1) uniform sampler2D in_normal_metal;
layout(binding = 2) uniform sampler2D in_depth;

layout(binding = 0) uniform Data
{
    FrameData frame;
};

layout(binding = 1) buffer PointLights
{
    PointLight point_lights[];
};

uniform uint light_index;

void main()
{
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const float depth = texelFetch(in_depth, coord, 0).r;

    if(depth <= 0.0)
    {
        discard;
    }

    const vec2 uv = gl_FragCoord.xy / vec2(textureSize(in_depth, 0));

    const vec4 albedo_rough = texelFetch(in_albedo_rough, coord, 0);
    const vec4 normal_metal = texelFetch(in_normal_metal, coord, 0);

    const vec3 base_color = albedo_rough.rgb;
    const float roughness = albedo_rough.a;
    const vec3 normal = normalize(normal_metal.rgb * 2.0 - 1.0);
    const float metal = normal_metal.a;

    const vec3 position = unproject(uv, depth, frame.camera.inv_view_proj);
    const vec3 view_dir = normalize(frame.camera.position - position);

    const PointLight light = point_lights[light_index];
    const vec3 to_light = light.position - position;
    const float dist = length(to_light);
    const vec3 light_vec = to_light / dist;

    const float att = attenuation(dist, light.radius);
    if(att <= 0.0)
    {
        discard;
    }

    out_color = vec4(eval_brdf(normal, view_dir, light_vec, base_color, metal, roughness) * att * light.color, 1.0);
}
