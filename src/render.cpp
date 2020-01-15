#include <cassert>
#include <iostream>
#include <algorithm>
#include <utility>
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <Eigen/Dense>
#include "render.hpp"

using Vector3fVec = std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>>;
using Vector3iVec = std::vector<Eigen::Vector3i, Eigen::aligned_allocator<Eigen::Vector3i>>;

SRender::FrameBuffer::FrameBuffer(int width, int height, int defaultZ)
: _image{width, height, TGAImage::RGB}, _zBuffer(width * height, defaultZ) {
}

void SRender::FrameBuffer::Write(const std::string path) {
  _image.flip_vertically();
  _image.write_tga_file(path.c_str());
}

int SRender::FrameBuffer::Test(int x, int y, int z) {
  if(x < 0 || y < 0 || x >= width() || y >= height()) {
    return -1;
  }
  
  int i = x + y * width();
  if(!enableDepthTest) {
    // Always able to write
    // Return the actual z value in case depth write is enabled
    return _zBuffer[i];
  } else {
    return _zBuffer[i] <= z ? i : -1;
  }
}

int SRender::FrameBuffer::Test(const Eigen::Vector3i &v) {
  return this->Test(v.x(), v.y(), v.z());
}

void SRender::FrameBuffer::Set(int x, int y, int z, TGAColor color) {
  int i = this->Test(x, y, z);
  if(i != -1) {
    _image.set(x, y, color);
    if(enableDepthWrite) _zBuffer[i] = z;
  }
}

void SRender::FrameBuffer::Set(const Eigen::Vector3i& v, TGAColor color) {
  this->Set(v.x(), v.y(), v.z(), color);
}

void SRender::FrameBuffer::SetCb(int x, int y, int z, std::function<TGAColor()>&& cb) {
  int i = this->Test(x, y, z);
  if(i > 0) {
    _image.set(x, y, cb());
    if(enableDepthWrite) _zBuffer[i] = z;
  }
}

void SRender::FrameBuffer::SetCb(const Eigen::Vector3i& v, std::function<TGAColor()>&& cb) {
  this->SetCb(v.x(), v.y(), v.z(), std::move(cb));
}

void SRender::Pipeline::BindShader(VCallback vsh) {
  this->_vsh = vsh;
}

void SRender::Pipeline::BindShader(FCallback fsh) {
  this->_fsh = fsh;
}

void SRender::Pipeline::BindShaders(VCallback vsh, FCallback fsh) {
  this->_vsh = vsh;
  this->_fsh = fsh;
}

inline Vector3iVec ApplyVertexShader(
  const Vector3fVec& vec,
  SRender::Pipeline::VCallback&& vsh
) {
  Vector3iVec res(vec.size());
  for(auto& v : vec) {
    res.push_back(vsh(v));
  }
  return res;
}

void SRender::Pipeline::DrawLines(
  VerticesVector verts,
  IndicesVector indices,
  FrameBuffer& frame
) {
  auto transformed = ApplyVertexShader(verts, std::forward<VCallback>(this->vsh()));
  for(size_t i = 0; i < indices.size(); i += 2) {
    auto v1 = transformed[i];
    auto v2 = transformed[i + 1];

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
      int y = v1.y() * (1.0f - t) + v2.y() * t;
      int z = v1.z() * (1.0f - t) + v2.z() * t;
      auto col = [&]() {
        return this->fsh()({x, y, z}, std::nullopt);
	  };
      if(steep) {
        frame.SetCb(y, x, z, col);
      } else {
        frame.SetCb(x, y, z, col);
      }
    }
  }
}

void SRender::Pipeline::DrawTriangles(
  VerticesVector verts,
  IndicesVector indices,
  FrameBuffer& frame
) {
  auto transformed = ApplyVertexShader(verts, std::forward<VCallback>(this->vsh()));
  for(size_t i = 0; i < transformed.size(); i += 3) {
    auto v1 = transformed[i];
    auto v2 = transformed[i + 1];
    auto v3 = transformed[i + 2];

    // Stands for "Boudning Box Vertex 1/2"
    Eigen::Vector2i bbv1{
      std::max(0, std::min({v1.x(), v2.x(), v3.x()})),
      std::max(0, std::min({v1.y(), v2.y(), v3.y()}))
    };
    Eigen::Vector2i bbv2{
      std::min(frame.width(), std::max({v1.x(), v2.x(), v3.x()})),
      std::min(frame.height(), std::max({v1.y(), v2.y(), v3.y()}))
    };

    for (int y = bbv1.y(); y <= bbv2.y(); ++y) {
      for (int x = bbv1.x(); x <= bbv2.x(); ++x) {
        if (SRender::PtInTriangle({x, y, 0}, v1, v2, v3)) {
          auto bc = SRender::Barycentric({x, y, 0}, v1, v2, v3);
          int z = v1.z() * bc.x() + v2.z() * bc.y() + v3.z() * bc.z();
          auto cb = [&]() {
		    return this->fsh()({x, y, z}, std::nullopt);
		  };
          frame.SetCb(x, y, z, cb);
        }
      }
    }
  }
}

void SRender::Pipeline::DrawTriangleStrip(
  VerticesVector verts,
  IndicesVector indices,
  FrameBuffer& frame
) {
  std::vector<int> trIdx((indices.size() - 2) * 3);
  int lastTwo = indices[0];
  int lastOne = indices[1];
  for(auto it = std::next(indices.begin(), 2); it != indices.end(); ++it) {
    trIdx.push_back(lastTwo);
    trIdx.push_back(lastOne);
    trIdx.push_back(*it);
    lastTwo = lastOne;
    lastOne = *it;
  }
  DrawTriangles(verts, trIdx, frame);
}

// TODO
// void SRender::Pipeline::DrawPolygon(
//   VerticesVector verts,
//   IndicesVector indices,
//   FrameBuffer &frame
// ) {
//   std::vector<int> trIdx((indices.size() - 2) * 3);

//   DrawTriangles(verts, trIdx, frame);
// }

SRender::Camera::Camera()
: _view(Eigen::Matrix4f::Identity()),
  _projection(Eigen::Matrix4f::Identity()),
  _viewport(Eigen::Matrix4f::Identity()) {
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

  _projection.setIdentity();
  _projection(3, 2) = -1.0f / (eye - center).norm();
}

void SRender::Camera::Viewport(int x, int y, int w, int h) {
  static const float d = 255.0f;
  static const float minD = 0.0f;
  _viewport.setIdentity();

  // Translation
  _viewport(0, 3) = x + w/2;
  _viewport(1, 3) = y + h/2;
  _viewport(2, 3) = d/2;

  // Scaling
  _viewport(0, 0) = w/2;
  _viewport(1, 1) = h/2;
  _viewport(2, 2) = d/2;
}

Eigen::Vector3f SRender::Camera::Transform(const Eigen::Vector3f& pt) {
  Eigen::Vector4f res = _viewport * _projection * _view * SRender::RegToAffine(pt);
  return res.head<3>() / res.w();
}

template <class N>
N SRender::Map(N value, N fromMin, N fromMax, N toMin, N toMax)  {
  return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
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

  return std::abs(u.z()) < 1
    ? Vec(-1, -1, -1)
    : Vec(1.0f - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z());
}

float Sign(const Eigen::Vector3i& p1, const Eigen::Vector3i& p2, const Eigen::Vector3i& p3) {
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

Eigen::Vector4f SRender::RegToAffine(const Eigen::Vector3f &v) {
  return {v.x(), v.y(), v.z(), 1};
}

Eigen::Vector3f SRender::AffineToReg(const Eigen::Vector4f &v) {
  return v.head<3>() / v.w();
}

void SRender::Debug::DumpZBufferConsole(SRender::FrameBuffer& target) {
  for (int x = 0; x < target.width(); ++x) {
    for (int y = 0; y < target.height(); ++y) {
      int z = target.zBuffer()[x + y * target.width()];
      std::cout << z << " ";
    }
    std::cout << "\n";
  }
}

void SRender::Debug::DumpZBufferTGASimple(SRender::FrameBuffer& target) {
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

void SRender::Debug::DumpZBufferTGAFull(SRender::FrameBuffer& target) {
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

void SRender::Raster::DrawLine(
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

void SRender::Raster::DrawTriangle(
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

void SRender::Raster::DrawPolygon(
  const std::vector<Eigen::Vector3i>& verts,
  SRender::FrameBuffer& frame,
  TGAColor color
) {
  assert(verts.size() >= 3);

  auto lastOne = &verts[1];
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    SRender::Raster::DrawTriangle(verts[0], *lastOne, *it, frame, color);
    lastOne = &*it;
  }
}
