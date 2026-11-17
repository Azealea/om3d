#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>

#include "Camera.h"
#include "Framebuffer.h"
#include "PointLight.h"
#include "SceneObject.h"

namespace OM3D
{

    enum class RenderPass
    {
        Shadow,
        ZPrepass,
        GBuffer,
        Lighting,
        PointLights,
        Transparent,
        Forward,
    };

    class Scene : NonMovable
    {
    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);
        static Result<MeshData> mesh_from_gltf(const std::string& file_name);

        void render(RenderPass pass, Program* program) const;

        BSphere bounding_sphere() const;
        glm::mat4 sun_view_proj() const;

        void add_object(SceneObject obj);
        void add_light(PointLight obj);

        Span<const SceneObject> objects() const;
        Span<const PointLight> point_lights() const;

        Camera& camera();
        const Camera& camera() const;

        void set_envmap(std::shared_ptr<Texture> env);
        void set_ibl_intensity(float intensity);

        void set_sun(float altitude, float azimuth, glm::vec3 color = glm::vec3(1.0f));
        void set_sun_bias(float bias);

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;

        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
        glm::vec3 _sun_color = glm::vec3(1.0f);
        float _sun_bias = 0.30f;

        std::shared_ptr<Texture> _envmap;
        float _ibl_intensity = 1.0f;
        Material _sky_material;
        Material _sun_material;
        Material _point_light_material;
        StaticMesh _light_sphere;

        Camera _camera;
    };

} // namespace OM3D

#endif // SCENE_H
