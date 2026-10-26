#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <Material.h>
#include <StaticMesh.h>
#include <glm/matrix.hpp>
#include <memory>

#include "Camera.h"

namespace OM3D
{

    class SceneObject
    {
    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr,
                    std::shared_ptr<Material> material = nullptr);

        void render() const;

        const Material& material() const;

        void set_transform(const glm::mat4& tr);
        const glm::mat4& transform() const;
        bool is_in_frustum(const glm::vec3& cam_pos,
                           const Frustum& frustum) const;

    private:
        glm::mat4 _transform = glm::mat4(1.0f);

        std::shared_ptr<StaticMesh> _mesh;
        std::shared_ptr<Material> _material;
    };

} // namespace OM3D

#endif // SCENEOBJECT_H
