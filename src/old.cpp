#include <vector>
#include "../external/custom/tgaimage.hpp"
#include "render.hpp"
#include "old.hpp"

void DrawLine(
  const Eigen::Vector3i& v1In,
  const Eigen::Vector3i& v2In,
  SRender::FrameBuffer& frame,
  TGAColor color
) {
  auto v1 = v1In;
  auto v2 = v2In;

  // Swapping is safe because pass-by-value
  bool steep = false;
  if (std::abs(v1.x() - v2.x()) < std::abs(v1.y() - v2.y())) {
    std::swap(v1.x(), v1.y());
    std::swap(v2.x(), v2.y());
    steep = true;
  }
  if(v1.x() > v2.x()) {
    std::swap(v1, v2);
  }
  for(int x = v1.x(); x <= v2.x(); ++x) {
    float t = (x - v1.x()) / static_cast<float>(v2.x() - v1.x());
    int y = v1.y() * (1.0 - t) + v2.y() * t;
    int z = v1.z() * (1.0 - t) + v2.z() * t;
    if(steep) {
      frame.Set(y ,x, z, color);
    } else {
      frame.Set(x, y, z, color);
    }
  }
}

void DrawTriangle(
  const Eigen::Vector3i& v1,
  const Eigen::Vector3i& v2,
  const Eigen::Vector3i& v3,
  SRender::FrameBuffer& frame,
  TGAColor color
) {
  // Stands for "Boudning Box Vertex 1/2"
  Eigen::Vector2i bbv1 {
    std::max(0, std::min({v1.x(), v2.x(), v3.x()})),
    std::max(0, std::min({v1.y(), v2.y(), v3.y()}))
  };
  Eigen::Vector2i bbv2 {
    std::min(frame.width(), std::max({v1.x(), v2.x(), v3.x()})),
    std::min(frame.height(), std::max({v1.y(), v2.y(), v3.y()}))
  };

  for(int y = bbv1.y(); y <= bbv2.y(); ++y) {
    for(int x = bbv1.x(); x <= bbv2.x(); ++x) {
      auto bc = SRender::Barycentric({x, y, 0}, v1, v2, v3);
      if(SRender::PtInTriangle({x, y, 0}, v1, v2, v3)) {
        float z =
          v1.z() * bc.x() +
          v2.z() * bc.y() +
          v3.z() * bc.z();
        frame.Set(x, y, z, color);
      }
    }
  }
}

void DrawTriangles(
  const std::vector<Eigen::Vector3i> &verts,
  const std::vector<int> indices,
  SRender::FrameBuffer &frame,
  TGAColor color
) {
  assert(verts.size() >= 3);
  assert(indices.size() % 3 == 0);

  for(int i = 0; i < indices.size(); i += 3) {
    DrawTriangle(verts[indices[i]], verts[indices[i + 1]], verts[indices[i + 2]], frame, color);
  }
}

void DrawPolygon(
  const std::vector<Eigen::Vector3i>& verts,
  SRender::FrameBuffer& frame,
  TGAColor color
) {
  assert(verts.size() >= 3);

  auto lastOne = &verts[1];
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    DrawTriangle(verts[0], *lastOne, *it, frame, color);
    lastOne = &*it;
  }
}
