#include "Scene.h"

#include <cmath>
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

#include "TypedBuffer.h"
#include "shader_structs.h"

namespace OM3D
{

    Scene::Scene()
    {
        _sky_material.set_program(Program::from_files("sky.frag", "screen.vert"));
        _sky_material.set_depth_test_mode(DepthTestMode::None);

        _sun_material.set_program(Program::from_files("deferred_lighting.frag", "screen.vert"));
        _sun_material.set_depth_test_mode(DepthTestMode::None);
        _sun_material.set_depth_write(false);

        _point_light_material.set_program(Program::from_files("deferred_point.frag", "screen.vert"));
        _point_light_material.set_blend_mode(BlendMode::Additive);
        _point_light_material.set_depth_test_mode(DepthTestMode::None);
        _point_light_material.set_depth_write(false);

        _envmap = std::make_shared<Texture>(Texture::empty_cubemap(4, ImageFormat::RGBA8_UNORM));
    }

    void Scene::add_object(SceneObject obj)
    {
        _objects.emplace_back(std::move(obj));
    }

    void Scene::add_light(PointLight obj)
    {
        _point_lights.emplace_back(std::move(obj));
    }

    Span<const SceneObject> Scene::objects() const
    {
        return _objects;
    }

    Span<const PointLight> Scene::point_lights() const
    {
        return _point_lights;
    }

    Camera& Scene::camera()
    {
        return _camera;
    }

    const Camera& Scene::camera() const
    {
        return _camera;
    }

    void Scene::set_envmap(std::shared_ptr<Texture> env)
    {
        _envmap = std::move(env);
    }

    void Scene::set_ibl_intensity(float intensity)
    {
        _ibl_intensity = intensity;
    }

    void Scene::set_sun_bias(float bias)
    {
        _sun_bias = bias;
    }

    void Scene::set_sun(float altitude, float azimuth, glm::vec3 color)
    {
        // Convert from degrees to radians
        const float alt = glm::radians(altitude);
        const float azi = glm::radians(azimuth);
        // Convert from polar to parmesian (cartesian for the pedantic)
        _sun_direction = glm::vec3(sin(azi) * cos(alt), sin(alt), cos(azi) * cos(alt));
        _sun_color = color;
    }

    BSphere Scene::bounding_sphere() const
    {
        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(std::numeric_limits<float>::lowest());

        for (const SceneObject& obj : _objects)
        {
            const BSphere sphere = obj.scene_bounding_sphere();
            min = glm::min(min, sphere.center - sphere.radius);
            max = glm::max(max, sphere.center + sphere.radius);
        }

        BSphere result;
        result.center = (min + max) * 0.5f;
        result.radius = glm::distance(result.center, max);
        return result;
    }

    glm::mat4 Scene::sun_view_proj() const
    {
        const BSphere sphere = bounding_sphere();
        const float r = sphere.radius;
        const glm::vec3 dir = glm::normalize(_sun_direction);
        const glm::vec3 up =
            std::abs(dir.y) > 0.99f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

        const glm::mat4 view = glm::lookAt(sphere.center + dir * r, sphere.center, up);
        const glm::mat4 proj = Camera::orthographic(-r, r, -r, r, 0.0f, 2.0f * r);

        return proj * view;
    }

    void Scene::render(RenderPass pass, Program* program) const
    {
        const bool lit = pass == RenderPass::Lighting || pass == RenderPass::PointLights
                         || pass == RenderPass::Forward;

        TypedBuffer<shader::FrameData> buffer(nullptr, 1);
        {
            auto mapping = buffer.map(AccessType::WriteOnly);
            if (pass == RenderPass::Shadow)
            {
                mapping[0].camera.view_proj = sun_view_proj();
            }
            else
            {
                mapping[0].camera.view_proj = _camera.view_proj_matrix();
                mapping[0].camera.inv_view_proj = glm::inverse(_camera.view_proj_matrix());
                mapping[0].camera.position = _camera.position();
            }

            if (lit)
            {
                mapping[0].point_light_count = u32(_point_lights.size());
                mapping[0].sun_color = _sun_color;
                mapping[0].sun_dir = glm::normalize(_sun_direction);
                mapping[0].ibl_intensity = _ibl_intensity;
                mapping[0].sun_view_proj = sun_view_proj();
                mapping[0].sun_bias = _sun_bias;
            }
        }
        buffer.bind(BufferUsage::Uniform, 0);

        TypedBuffer<shader::PointLight> light_buffer(nullptr,
                                                     std::max(_point_lights.size(), size_t(1)));
        if (lit)
        {
            {
                auto mapping = light_buffer.map(AccessType::WriteOnly);
                for (size_t i = 0; i != _point_lights.size(); ++i)
                {
                    const auto& light = _point_lights[i];
                    mapping[i] = { light.position(), light.radius(), light.color(), 0.0f };
                }
            }
            light_buffer.bind(BufferUsage::Storage, 1);

            DEBUG_ASSERT(_envmap && !_envmap->is_null());
            _envmap->bind(4);
            brdf_lut().bind(5);
        }

        if (pass == RenderPass::Lighting)
        {
            _sun_material.bind(false, nullptr);
            draw_full_screen_triangle();
            return;
        }

        if (pass == RenderPass::PointLights)
        {
            Program& light_program = _point_light_material.bind(false, nullptr);
            for (size_t i = 0; i != _point_lights.size(); ++i)
            {
                light_program.set_uniform(HASH("light_index"), u32(i));
                draw_full_screen_triangle();
            }
            glDisable(GL_BLEND);
            return;
        }

        if (pass == RenderPass::Forward)
        {
            _sky_material.bind(false, nullptr);
            _sky_material.set_uniform(HASH("intensity"), _ibl_intensity);
            draw_full_screen_triangle();
        }

        const bool cull = pass != RenderPass::Shadow;
        const Frustum frustum = _camera.build_frustum();
        const glm::vec3 cam_pos = _camera.position();

        for (const SceneObject& obj : _objects)
        {
            if (obj.material().is_opaque() && (!cull || obj.is_in_frustum(cam_pos, frustum)))
            {
                obj.render(pass == RenderPass::Forward, program);
            }
        }

        if (pass == RenderPass::Forward)
        {
            for (const SceneObject& obj : _objects)
            {
                if (!obj.material().is_opaque() && obj.is_in_frustum(cam_pos, frustum))
                {
                    obj.render(false, nullptr);
                }
            }
        }
    }

} // namespace OM3D
