#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include "../external/custom/tgaimage.hpp"
#include "render.hpp"

srd::Camera::Camera(Eigen::Vector3f pos, Eigen::Vector3f angle)
: _pos(pos), _angle(angle) {
}

srd::FrameBuffer::FrameBuffer(int width, int height, int defaultZ)
: _cam{}, _image{width, height, TGAImage::RGB}, _zBuffer(width * height, defaultZ) {
}

void srd::FrameBuffer::Write(const std::string path) {
  _image.flip_vertically();
  _image.write_tga_file(path.c_str());
}

void srd::FrameBuffer::Set(int x, int y, int z, TGAColor color) {
  if (x < 0 || y < 0 || x >= width() || y >= height()) {
    return;
  }

  const int i = x + y * width();
  if(!enableDepthTest || _zBuffer[i] <= z) {
    _image.set(x, y, color);
    if(enableDepthWrite) _zBuffer[i] = z;
  }
}

void srd::FrameBuffer::Set(const Eigen::Vector3i& v, TGAColor color) {
  this->Set(v.x(), v.y(), v.z(), color);
}

const Eigen::Vector3i& srd::PtMin(const Eigen::Vector3i& p1, const Eigen::Vector3i& p2) {
  if(p1.x() > p2.x()) return p2;
  if(p1.y() > p2.y()) return p2;
  return p1;
}
const Eigen::Vector3i& srd::PtMax(const Eigen::Vector3i& p1, const Eigen::Vector3i& p2) {
  if(p1.x() < p2.x()) return p2;
  if(p1.y() < p2.y()) return p2;
  return p1;
}

void srd::DrawLine(
  const Eigen::Vector3i& v1In,
  const Eigen::Vector3i& v2In,
  FrameBuffer& frame,
  TGAColor color
) {
  Eigen::Vector3i v1(v1In);
  Eigen::Vector3i v2(v2In);

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

void srd::DrawTriangle(
  const Eigen::Vector3i& v1,
  const Eigen::Vector3i& v2,
  const Eigen::Vector3i& v3,
  FrameBuffer& frame,
  TGAColor color
) {
  // Stands for "Boudning Box Vertex 1/2"
  Eigen::Vector2i bbv1 {std::min({v1.x(), v2.x(), v3.x()}), std::min({v1.y(), v2.y(), v3.y()})};
  Eigen::Vector2i bbv2 {std::max({v1.x(), v2.x(), v3.x()}), std::max({v1.y(), v2.y(), v3.y()})};

  for(int y = bbv1.y(); y <= bbv2.y(); ++y) {
    for(int x = bbv1.x(); x <= bbv2.x(); ++x) {
      Eigen::Vector3f bc = srd::Barycentric({x, y, 0}, v1, v2, v3);
      float z =
        v1.z() * bc.x() +
        v2.z() * bc.y() +
        v3.z() * bc.z();
      if(srd::PtInTriangle({x, y, 0}, v1, v2, v3)) {
        frame.Set(x, y, z, color);
      }
    }
  }
}

void srd::DrawTriangles(
  const std::vector<Eigen::Vector3i> &verts,
  const std::vector<int> indices,
  FrameBuffer &frame,
  TGAColor color
) {
  assert(verts.size() >= 3);
  assert(indices.size() % 3 == 0);

  for(int i = 0; i < indices.size(); i += 3) {
    srd::DrawTriangle(verts[indices[i]], verts[indices[i + 1]], verts[indices[i + 2]], frame, color);
  }
}

void srd::DrawTriangleStrip(
  const std::vector<Eigen::Vector3i> &verts,
  FrameBuffer &frame,
  TGAColor color
) {
  assert(verts.size() >= 3);

  auto lastOne(verts[0]);
  auto lastTwo(verts[1]);
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    srd::DrawTriangle(lastOne, lastTwo, *it, frame, color);
    lastTwo = lastOne;
    lastOne = *it;
  }
}

void srd::DrawPolygon(
  const std::vector<Eigen::Vector3i>& verts,
  FrameBuffer& frame,
  TGAColor color
) {
  assert(verts.size() >= 3);

  auto lastOne(verts[1]);
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    srd::DrawTriangle(verts[0], lastOne, *it, frame, color);
    lastOne = *it;
  }
}

template <class N>
N srd::Map(N value, N fromMin, N fromMax, N toMin, N toMax)  {
  return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
}

Eigen::Vector3f srd::Barycentric(
  const Eigen::Vector3i& pt,
  const Eigen::Vector3i& v1,
  const Eigen::Vector3i& v2,
  const Eigen::Vector3i& v3
) {
  Eigen::Vector3f u =
           Eigen::Vector3f(v3.x() - v1.x(), v2.x() - v1.x(), v1.x() - pt.x())
    .cross(Eigen::Vector3f(v3.y() - v1.y(), v2.y() - v1.y(), v1.y() - pt.y()));
  if (std::abs(u.z()) < 1)
    return Eigen::Vector3f(-1, 1, 1);
  return Eigen::Vector3f(1.0f - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z());
}

float Sign(const Eigen::Vector3i& p1, const Eigen::Vector3i& p2, Eigen::Vector3i p3) {
  return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
}
bool srd::PtInTriangle(
  const Eigen::Vector3i& pt,
  const Eigen::Vector3i& v1,
  const Eigen::Vector3i& v2,
  const Eigen::Vector3i& v3
) {
  float d1 = Sign(pt, v1, v2);
  float d2 = Sign(pt, v2, v3);
  float d3 = Sign(pt, v3, v1);
  bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
  return !(hasNeg && hasPos);
}

void srd::debug::DumpZBufferConsole(srd::FrameBuffer& target) {
  for (int x = 0; x < target.width(); ++x) {
    for (int y = 0; y < target.height(); ++y) {
      int z = target.zBuffer()[x + y * target.width()];
      std::cout << z << " ";
    }
    std::cout << "\n";
  }
}

void srd::debug::DumpZBufferTGA(srd::FrameBuffer& target) {
  srd::FrameBuffer frame(target.width(), target.height(), 0);
  frame.SetDepthTest(false);
  frame.setDepthWrite(false);

  int32_t minZ = INT32_MAX;
  int32_t maxZ = INT32_MIN;
  for (int x = 0; x < target.width(); ++x) {
    for (int y = 0; y < target.height(); ++y) {
      int z = target.zBuffer()[x + y * target.width()];
      minZ = std::min(minZ, z);
      maxZ = std::max(maxZ, z);
    }
  }
  if(minZ == maxZ) return;
  for (int x = 0; x < target.width(); ++x) {
    for (int y = 0; y < target.height(); ++y) {
      int z = target.zBuffer()[x + y * target.width()];
      int c = srd::Map(z, minZ, maxZ, 0, 255);
      frame.Set(x, y, 0, TGAColor(c, c, c, 255));
    }
  }
  frame.Write("./build/zdump.tga");
}
