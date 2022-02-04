#pragma once

#include "Color.hpp"

#include <functional>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    RgbaColor color;

    bool operator==(const Vertex&) const = default;
};

template <>
struct std::hash<Vertex> {
    size_t operator()(const Vertex& vert) const;
};

struct Line {
    glm::vec3 vertices[2];
};

struct Triangle {
    glm::vec3 vertices[3];

    static glm::vec3 CalcBarycentric(const glm::vec3& pt, const glm::vec3 vertices[3]);
    glm::vec3 CalcBarycentric(const glm::vec3& pt) const;

    // Note: ignores Z value
    static bool ContainsPoint(glm::vec2 pt, const glm::vec3 vertices[3]);
    bool ContainsPoint(glm::vec2 pt) const;
};
