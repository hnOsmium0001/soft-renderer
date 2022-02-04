#include "Primitive.hpp"

#include "Math.hpp"
#include <glm/gtx/hash.hpp>

size_t std::hash<Vertex>::operator()(const Vertex& vert) const {
    size_t seed = 0;
    seed ^= std::hash<glm::vec3>()(vert.pos) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<glm::vec3>()(vert.normal) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<glm::vec2>()(vert.uv) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

glm::vec3 Triangle::CalcBarycentric(const glm::vec3& pt, const glm::vec3 vertices[3]) {
    auto& v1 = vertices[0];
    auto& v2 = vertices[1];
    auto& v3 = vertices[2];

    auto u = glm::cross(
        glm::vec3(v3.x - v1.x, v2.x - v1.x, v1.x - pt.x),
        glm::vec3(v3.y - v1.y, v2.y - v1.y, v1.y - pt.y));

    return MathUtils::Abs(u.z) < 1
        ? glm::vec3(-1, -1, -1)
        : glm::vec3(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

glm::vec3 Triangle::CalcBarycentric(const glm::vec3& pt) const {
    return CalcBarycentric(pt, vertices);
}

static float Sign(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool Triangle::ContainsPoint(glm::vec2 pt, const glm::vec3 vertices[3]) {
    float d1 = ::Sign(glm::vec3(pt, 0.0f), vertices[0], vertices[1]);
    float d2 = ::Sign(glm::vec3(pt, 0.0f), vertices[1], vertices[2]);
    float d3 = ::Sign(glm::vec3(pt, 0.0f), vertices[2], vertices[0]);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(hasNeg && hasPos);
}

bool Triangle::ContainsPoint(glm::vec2 pt) const {
    return ContainsPoint(pt, vertices);
}
