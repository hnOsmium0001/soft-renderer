#include "Rasterizer.hpp"

#include "Color.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Scene.hpp"

FrameBuffer::FrameBuffer()
    : dimensions{ 0, 0 } {
}

FrameBuffer::FrameBuffer(Size2<int> dimensions) {
    Resize(dimensions);
}

void FrameBuffer::Resize(Size2<int> dimensions) {
    this->dimensions = dimensions;
    pixels.resize(dimensions.Area(), RgbaColor(0, 0, 0));
    depths.resize(dimensions.Area());
}

RgbaColor FrameBuffer::GetPixel(glm::ivec2 pos) const {
    return pixels[pos.y * dimensions.width + pos.x];
}

void FrameBuffer::SetPixel(glm::ivec2 pos, float z, RgbaColor color) {
    int idx = pos.y * dimensions.width + pos.x;
    if (depths[idx] < z) {
        pixels[idx] = color;
        depths[idx] = z;
    }
}

void Rasterizer::DrawLine(const glm::vec3 vertices[2], RgbaColor color) {
    auto a = vertices[0];
    auto b = vertices[1];

    bool steep = false;
    if (std::abs(a.x - b.x) < std::abs(a.y - b.y)) {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
        steep = true;
    }
    if (a.x > b.x) {
        std::swap(a, b);
    }

    int start = a.x;
    int end = b.x;
    for (int x = start; x <= end; ++x) {
        float t = (x - a.x) / (b.x - a.x);
        int y = a.y * (1.0f - t) + b.y * t;
        float z = a.z * (1.0f - t) + b.z * t;
        if (steep) {
            framebuffer->SetPixel({ y, x }, z, color);
        } else {
            framebuffer->SetPixel({ x, y }, z, color);
        }
    }
}

void Rasterizer::DrawTriangle(const glm::vec3 vertices[3], const RgbaColor colors[3]) {
    auto t0 = vertices[0];
    auto t1 = vertices[1];
    auto t2 = vertices[2];

#if 0
    // TODO fix z and color
    // Sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!)
    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);

    int total_height = t2.y - t0.y;
    for (int y = t0.y; y <= t1.y; y++) {
        int segment_height = t1.y - t0.y + 1;
        float alpha = (float)(y - t0.y) / total_height;
        float beta = (float)(y - t0.y) / segment_height; // be careful with divisions by zero
        glm::ivec2 A = t0 + (t2 - t0) * alpha;
        glm::ivec2 B = t0 + (t1 - t0) * beta;
        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j <= B.x;) {
            // Attention, due to int casts t0.y+i != A.yj++) {
            framebuffer->SetPixel({ j, y }, 0.0f, colors[0]);
        }
    }
    for (int y = t1.y; y <= t2.y; y++) {
        int segment_height = t2.y - t1.y + 1;
        float alpha = (float)(y - t0.y) / total_height;
        float beta = (float)(y - t1.y) / segment_height; // be careful with divisions by zero
        glm::ivec2 A = t0 + (t2 - t0) * alpha;
        glm::ivec2 B = t1 + (t2 - t1) * beta;
        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j <= B.x; j++) {
            // Attention, due to int casts t0.y+i != A.y
            framebuffer->SetPixel({ j, y }, 0.0f, colors[0]);
        }
    }
#else
    glm::ivec2 bbv1{
        std::max(0.0f, std::min({ t0.x, t1.x, t2.x })),
        std::max(0.0f, std::min({ t0.y, t1.y, t2.y }))
    };
    glm::ivec2 bbv2{
        std::min<int>(framebuffer->dimensions.width, std::max({ t0.x, t1.x, t2.x })),
        std::min<int>(framebuffer->dimensions.height, std::max({ t0.y, t1.y, t2.y }))
    };

    for (int y = bbv1.y; y <= bbv2.y; ++y) {
        for (int x = bbv1.x; x <= bbv2.x; ++x) {
            auto bc = Triangle::CalcBarycentric(glm::vec3(x, y, 0.0f), vertices);
            if (bc.x >= 0 && bc.x <= 1 && bc.y >= 0 && bc.y <= 1) {
                float bcX = t0.x * bc.x + t1.x * bc.y + t2.x * bc.z;
                float bcY = t0.y * bc.x + t1.y * bc.y + t2.y * bc.z;
                float bcZ = t0.z * bc.x + t1.z * bc.y + t2.z * bc.z;

                RgbaColor color(
                    colors[0].r * bc.x + colors[1].r * bc.y + colors[2].r * bc.z,
                    colors[0].g * bc.x + colors[1].g * bc.y + colors[2].g * bc.z,
                    colors[0].b * bc.x + colors[1].b * bc.y + colors[2].b * bc.z,
                    colors[0].a * bc.x + colors[1].a * bc.y + colors[2].a * bc.z);

                framebuffer->SetPixel({ x, y }, bcZ, color);
            }
        }
    }
#endif
}

void Rasterizer::DrawRectangle(const Rect<float>& rect, float z) {
    // TODO
}

void Rasterizer::DrawMesh(const Camera& camera, const Mesh& mesh) {
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        glm::vec3 positions[] = {
            mesh.vertices[i].pos,
            mesh.vertices[i + 1].pos,
            mesh.vertices[i + 2].pos,
        };
        RgbaColor colors[] = {
            mesh.vertices[i].color,
            mesh.vertices[i + 1].color,
            mesh.vertices[i + 2].color,
        };

        DrawTriangle(positions, colors);
    }
}
