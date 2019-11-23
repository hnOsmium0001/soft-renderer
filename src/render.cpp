#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cstdint>
#include "../external/custom/tgaimage.hpp"
#include "render.hpp"

SRender::FrameBuffer::FrameBuffer(int width, int height, int defaultZ)
: _image{width, height, TGAImage::RGB}, _zBuffer(width * height, defaultZ) {
}

void SRender::FrameBuffer::Write(const std::string path) {
  _image.flip_vertically();
  _image.write_tga_file(path.c_str());
}

void SRender::FrameBuffer::Set(int x, int y, int z, TGAColor color) {
  if (x < 0 || y < 0 || x >= width() || y >= height()) {
    return;
  }

  const int i = x + y * width();
  if(!enableDepthTest || _zBuffer[i] <= z) {
    _image.set(x, y, color);
    if(enableDepthWrite) _zBuffer[i] = z;
  }
}

void SRender::FrameBuffer::Set(const Eigen::Vector3i& v, TGAColor color) {
  this->Set(v.x(), v.y(), v.z(), color);
}

void SRender::DrawLine(
  const Eigen::Vector3i& v1In,
  const Eigen::Vector3i& v2In,
  FrameBuffer& frame,
  TGAColor color
) {
  auto v1(v1In);
  auto v2(v2In);

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

void SRender::DrawTriangle(
  const Eigen::Vector3i& v1,
  const Eigen::Vector3i& v2,
  const Eigen::Vector3i& v3,
  FrameBuffer& frame,
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
      float z =
        v1.z() * bc.x() +
        v2.z() * bc.y() +
        v3.z() * bc.z();
      if(SRender::PtInTriangle({x, y, 0}, v1, v2, v3)) {
        // std::cout << Eigen::Vector3i(x, y, z) << "\n\n";
        frame.Set(x, y, z, color);
      }
    }
  }
}

void SRender::DrawTriangles(
  const std::vector<Eigen::Vector3i> &verts,
  const std::vector<int> indices,
  FrameBuffer &frame,
  TGAColor color
) {
  assert(verts.size() >= 3);
  assert(indices.size() % 3 == 0);

  for(int i = 0; i < indices.size(); i += 3) {
    SRender::DrawTriangle(verts[indices[i]], verts[indices[i + 1]], verts[indices[i + 2]], frame, color);
  }
}

void SRender::DrawTriangleStrip(
  const std::vector<Eigen::Vector3i> &verts,
  FrameBuffer &frame,
  TGAColor color
) {
  assert(verts.size() >= 3);

  auto lastOne(verts[0]);
  auto lastTwo(verts[1]);
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    SRender::DrawTriangle(lastOne, lastTwo, *it, frame, color);
    lastTwo = lastOne;
    lastOne = *it;
  }
}

void SRender::DrawPolygon(
  const std::vector<Eigen::Vector3i>& verts,
  FrameBuffer& frame,
  TGAColor color
) {
  assert(verts.size() >= 3);

  auto lastOne(verts[1]);
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    SRender::DrawTriangle(verts[0], lastOne, *it, frame, color);
    lastOne = *it;
  }
}

SRender::Camera::Camera(Eigen::Vector3f pos, Eigen::Vector3f up)
: _view(Eigen::Matrix4f::Identity()), _projection(Eigen::Matrix4f::Identity()), _viewport(Eigen::Matrix4f::Identity()) {
}

void SRender::Camera::LookAt(
  const Eigen::Vector3f& eye,
  const Eigen::Vector3f& up,
  const Eigen::Vector3f& center
) {
  Eigen::Vector3f z = (eye - center).normalized();
  Eigen::Vector3f x = up.cross(z).normalized();
  Eigen::Vector3f y = z.cross(x).normalized();

  // x x x -c
  // y y y -c
  // z z z -c
  // 0 0 0  1
  _view.setIdentity();
  _view.block<1, 3>(0, 0) = x;
  _view.block<1, 3>(1, 0) = y;
  _view.block<1, 3>(2, 0) = z;
  _view.block<3, 1>(0, 3) = -center;

  _projection(3, 2) = -1.0f / (eye - center).norm();
}

void SRender::Camera::Viewport(int x, int y, int w, int h) {
  static const int d = 255;
  _viewport.setIdentity();

  // Translation
  _viewport(0, 3) = x + w/2;
  _viewport(1, 3) = y + h/2;
  _viewport(2, 3) = d/2;

  // Scaling
  _viewport(0, 0) = w/2;
  _viewport(1, 1) = y/2;
  _viewport(2, 2) = d/2;
}

Eigen::Vector3i SRender::Camera::ToScreen(const Eigen::Vector3f& pt) {
  // std::cout << pt << "\n\n";
  Eigen::Vector4f affine = Eigen::Vector4f();
  affine.block<3, 1>(0, 0) = pt;
  Eigen::Vector4f res = _viewport * _projection * _view * affine;
  Eigen::Vector3f unaffine = res.head<3>() / res.w();
  // std::cout << unaffine << "\n\n";
  return unaffine.cast<int>();
}

template <class V>
Eigen::Matrix<V, 3, 1> SRender::Barycentric(
  const Eigen::Matrix<V, 3, 1>& pt,
  const Eigen::Matrix<V, 3, 1>& v1,
  const Eigen::Matrix<V, 3, 1>& v2,
  const Eigen::Matrix<V, 3, 1>& v3
) {
  using Vec = Eigen::Matrix<V, 3, 1>;
  Vec u = Vec(v3.x() - v1.x(), v2.x() - v1.x(), v1.x() - pt.x())
    .cross(Vec(v3.y() - v1.y(), v2.y() - v1.y(), v1.y() - pt.y()));

  if (std::abs(u.z()) < 1)
    return Vec(-1, 1, 1);
  else
    return Vec(1.0f - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z());
}

float Sign(const Eigen::Vector3i& p1, const Eigen::Vector3i& p2, Eigen::Vector3i p3) {
  return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
}
template <class V>
bool SRender::PtInTriangle(
  const Eigen::Matrix<V, 3, 1>& pt,
  const Eigen::Matrix<V, 3, 1>& v1,
  const Eigen::Matrix<V, 3, 1>& v2,
  const Eigen::Matrix<V, 3, 1>& v3
) {
  float d1 = Sign(pt, v1, v2);
  float d2 = Sign(pt, v2, v3);
  float d3 = Sign(pt, v3, v1);
  bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
  return !(hasNeg && hasPos);
}

template <class N>
N SRender::Map(N value, N fromMin, N fromMax, N toMin, N toMax)  {
  return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
}

void SRender::debug::DumpZBufferConsole(SRender::FrameBuffer& target) {
  for (int x = 0; x < target.width(); ++x) {
    for (int y = 0; y < target.height(); ++y) {
      int z = target.zBuffer()[x + y * target.width()];
      std::cout << z << " ";
    }
    std::cout << "\n";
  }
}

void SRender::debug::DumpZBufferTGASimple(SRender::FrameBuffer& target) {
  SRender::FrameBuffer frame(target.width(), target.height(), 0);
  frame.SetDepthTest(false);
  frame.setDepthWrite(false);
  for (int x = 0; x < target.width(); ++x) {
    for (int y = 0; y < target.height(); ++y) {
      int z = target.zBuffer()[x + y * target.width()];
      frame.Set(x, y, 0, TGAColor(z, z, z, 255));
    }
  }
  frame.Write("./build/zdump.tga");
}

void SRender::debug::DumpZBufferTGAFull(SRender::FrameBuffer& target) {
  SRender::FrameBuffer frame(target.width(), target.height(), 0);
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
  if(minZ == maxZ) goto end;
  for (int x = 0; x < target.width(); ++x) {
    for (int y = 0; y < target.height(); ++y) {
      int z = target.zBuffer()[x + y * target.width()];
      int c = SRender::Map(z, minZ, maxZ, 0, 255);
      frame.Set(x, y, 0, TGAColor(c, c, c, 255));
    }
  }

  end:
  frame.Write("./build/zdump.tga");
}
