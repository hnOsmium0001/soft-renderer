#pragma once

#include "Renderer/Primitive.hpp"

#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <iosfwd>
#include <string_view>
#include <vector>

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

public:
    void ReadObj(std::istream& data);
    void ReadObjAt(const char* path);
};
