#include "SceneObject.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "BSphere.h"
#include "glm/ext/vector_float3.hpp"

namespace OM3D
{

    SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh,
                             std::shared_ptr<Material> material)
        : _mesh(std::move(mesh))
        , _material(std::move(material))
    {}

    void SceneObject::render() const
    {
        if (!_material || !_mesh)
        {
            return;
        }

        _material->set_uniform(HASH("model"), transform());
        _material->bind();
        _mesh->draw();
    }

    const Material& SceneObject::material() const
    {
        DEBUG_ASSERT(_material);
        return *_material;
    }

    void SceneObject::set_transform(const glm::mat4& tr)
    {
        _transform = tr;
    }

    const glm::mat4& SceneObject::transform() const
    {
        return _transform;
    }

    static bool above_plane(const glm::vec3& plane_pos,
                            const glm::vec3& plane_normal,
                            const glm::vec3& center, float radius)
    {
        glm::vec3 between = center - plane_pos;
        float distance = glm::dot(between, plane_normal); // projected distance
        return distance >= -radius;
    }

    bool SceneObject::is_in_frustum(const glm::vec3& cam_pos,
                                    const Frustum& frustum) const
    {
        const glm::vec3 center =
            glm::vec3(_transform * glm::vec4(_mesh->bSphere.center, 1.0f));

        const float radius = _mesh->bSphere.radius
            * std::max({ glm::length(glm::vec3(_transform[0])),
                         glm::length(glm::vec3(_transform[1])),
                         glm::length(glm::vec3(_transform[2])) });

        for (const glm::vec3& normal :
             { frustum._near_normal, frustum._top_normal,
               frustum._bottom_normal, frustum._right_normal,
               frustum._left_normal })
        {
            if (!above_plane(cam_pos, normal, center, radius))
            {
                return false;
            }
        }

        return true;
    }

} // namespace OM3D
