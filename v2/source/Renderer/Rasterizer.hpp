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

    struct RefreshOp {
        Size2<int> newDim = { 0, 0 };
        RgbaColor color = RgbaColor(0, 0, 0);
        float depth = 0.0f;
    };
    void Refresh(const RefreshOp& op);

#if 1 // Specialized functions for refershing part of the framebuffer
    void Resize(Size2<int> dimensions);
    void ClearColor(RgbaColor color);
    void ClearDepth(float depth);
#endif

    RgbaColor GetPixel(glm::ivec2 pos) const;
    void SetPixel(glm::ivec2 pos, float z, RgbaColor color);
};

class Rasterizer {
public:
    FrameBuffer* framebuffer;

public:
    FrameBuffer* GetTarget() const;
    void SetTarget(FrameBuffer* framebuffer);

    void DrawLine(const glm::vec3 vertices[2], RgbaColor color);

    void DrawTriangle(const glm::vec3 vertices[3], const RgbaColor colors[3]);

    // Helper for axis-aligned rectangles.
    // Increases rendering performance, compared to calling DrawTriangle twice
    void DrawRectangle(const Rect<float>& rect, float z = 0.0f);

    void DrawMesh(const Camera& camera, const Mesh& mesh);
};
