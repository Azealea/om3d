#ifndef BSPHERE_H
#define BSPHERE_H

#include <Vertex.h>
#include <graphics.h>
#include <vector>

namespace OM3D
{
    class BSphere
    {
    public:
        BSphere() = default;
        BSphere(const std::vector<Vertex>& points);

        glm::vec3 center = glm::vec3(0.0f);
        float radius = 0.0f;
    };
} // namespace OM3D

#endif // BSPHERE_H
