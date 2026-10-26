
#include "BSphere.h"

#include <algorithm>
#include <glm/geometric.hpp>
#include <vector>

#include "Vertex.h"
#include "glm/ext/vector_float3.hpp"

namespace OM3D
{
    static glm::vec3 farthest_point(const std::vector<Vertex>& points,
                                    glm::vec3 from)
    {
        auto vertex = std::max_element(
            points.begin(), points.end(), [&](auto const& a, auto const& b) {
                return glm::distance(a.position, from)
                    < glm::distance(b.position, from);
            });
        return vertex->position;
    }

    BSphere::BSphere(const std::vector<Vertex>& points)
    {
        // ritter 's bounding sphere
        if (points.size() == 0)
        {
            return;
        }

        glm::vec3 p = points[0].position;

        glm::vec3 y = farthest_point(points, p);
        glm::vec3 z = farthest_point(points, y);

        radius = glm::distance(y, z) * 0.5f;
        center = (z + y) * 0.5f;

        for (auto const& v : points)
        {
            const float dist = glm::distance(v.position, center);
            if (dist > radius)
            {
                const float new_radius = (radius + dist) * 0.5f;
                center += (v.position - center) * ((dist - new_radius) / dist);
                radius = new_radius;
            }
        }
    }
} // namespace OM3D
