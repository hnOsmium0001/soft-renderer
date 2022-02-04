#pragma once

#include "Color.hpp"
#include "Rect.hpp"
#include "Size.hpp"
#include "all_fwd.hpp"

#include <vector>

class FrameBuffer {
public:
    // Row-major
    std::vector<RgbaColor> pixels;
    std::vector<float> depths;
    Size2<int> dimensions;

public:
    FrameBuffer();
    FrameBuffer(Size2<int> dimensions);

    void Resize(Size2<int> dimensions);

    RgbaColor GetPixel(glm::ivec2 pos) const;
    void SetPixel(glm::ivec2 pos, float z, RgbaColor color);
};

class Rasterizer {
public:
    FrameBuffer* framebuffer;

public:
    FrameBuffer& GetTarget() const;
    void SetTarget(FrameBuffer& framebuffer);

    void DrawLine(const glm::vec3 vertices[2], RgbaColor color);

    void DrawTriangle(const glm::vec3 vertices[3], const RgbaColor colors[3]);

    // Helper for axis-aligned rectangles.
    // Increases rendering performance, compared to calling DrawTriangle twice
    void DrawRectangle(const Rect<float>& rect, float z = 0.0f);

    void DrawMesh(const Camera& camera, const Mesh& mesh);
};
