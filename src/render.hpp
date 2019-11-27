#pragma once

#include <vector>
#include <memory>
#include "../external/eigen/Dense"
#include "../external/custom/tgaimage.hpp"

// TODO use std::variant
// #if __cplusplus != 201703L
#include "../external/variant/mapbox/variant.hpp"
// #endif

// Stands for "Soft RenDerer"
namespace SRender {

  // 2D rendering

  class FrameBuffer {
  private:
    TGAImage _image;
    std::vector<int> _zBuffer;

    bool enableDepthTest = true;
    bool enableDepthWrite = true;

  public:
    FrameBuffer(int width, int height, int defaultZ=0);
    void Write(const std::string path);
    void Set(int x, int y, int z, TGAColor color);
    void Set(const Eigen::Vector3i& v, TGAColor color);

    void SetDepthTest(bool enable) { enableDepthTest = enable; }
    void setDepthWrite(bool enable) { enableDepthWrite = enable; }

    // Getters
    int width() { return _image.get_width(); }
    int height() { return _image.get_width(); }
    TGAImage& image() { return this->_image; }
    const TGAImage& image() const { return this->_image; }
    std::vector<int>& zBuffer() { return this->_zBuffer; }
    const std::vector<int>& zBuffer() const { return this->_zBuffer; }
  };

  void DrawLine(const Eigen::Vector3i& v1, const Eigen::Vector3i& v2, FrameBuffer& frame, TGAColor color);
  void DrawTriangle(const Eigen::Vector3i& v1, const Eigen::Vector3i& v2, const Eigen::Vector3i& v3, FrameBuffer& frame, TGAColor color);
  void DrawTriangles(const std::vector<Eigen::Vector3i>& verts, const std::vector<int> indices, FrameBuffer& frame, TGAColor color);
  void DrawTriangleStrip(const std::vector<Eigen::Vector3i>& verts, FrameBuffer& frame, TGAColor color);
  void DrawPolygon(const std::vector<Eigen::Vector3i>& verts, FrameBuffer& frame, TGAColor color);

  // Render pipline for 2D and 3D rendering

  class LinePrimitive {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    std::array<Eigen::Vector3i, 2> verts;
  };

  class TrianglePrimitive {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    std::array<Eigen::Vector3i, 3> verts;
  };

  using ScrnPrimitive = mapbox::util::variant<LinePrimitive, TrianglePrimitive>;

  class ScrnPixel {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Eigen::Vector3i pos;
    TGAColor color;
  };

  class Pipeline {
  private:
    using VCallback = std::function<Eigen::Vector4f(const Eigen::Vector3f&)>;
    using FCallback = std::function<ScrnPixel(const Eigen::Vector4f&)>;

    VCallback _vsh;
    FCallback _fsh;

  public:
    Pipeline() = default;
    void BindShader(VCallback vsh);
    void BindShader(FCallback fsh);
    void BindShaders(VCallback vsh, FCallback fsh);
    void Render(FrameBuffer& frame, const std::vector<ScrnPrimitive>& primitives);
    ScrnPixel ToViewport(const ScrnPrimitive& primitive, const Eigen::Vector3f& pt);

    VCallback& vsh() { return _vsh; }
    const VCallback& vsh() const { return _vsh; }
    FCallback& fsh() { return _fsh; }
    const FCallback& fsh() const { return _fsh; }
  };

  // 3D specific rendering

  class Camera {
  private:
    Eigen::Matrix4f _view;
    Eigen::Matrix4f _projection;
    Eigen::Matrix4f _viewport;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Camera(Eigen::Vector3f pos = {0, 0, -1}, Eigen::Vector3f up = {0, 1, 0});
    void LookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& up, const Eigen::Vector3f& center);
    void Viewport(int x, int y, int w, int h);
    Eigen::Vector3f Transform(const Eigen::Vector3f& pt);

    Eigen::Matrix4f& view() { return _view; }
    const Eigen::Matrix4f& view() const { return _view; }
    Eigen::Matrix4f& projection() { return _projection; }
    const Eigen::Matrix4f& projection() const { return _projection; }
    Eigen::Matrix4f& viewport() { return _viewport; }
    const Eigen::Matrix4f& viewport() const { return _viewport; }
  };

  // Misc utilities

  template <class N>
  N Map(N value, N fromMin, N fromMax, N toMin, N toMax);

  template <class V>
  Eigen::Matrix<V, 3, 1> Barycentric(const Eigen::Matrix<V, 3, 1>& pt, const Eigen::Matrix<V, 3, 1>& v1, const Eigen::Matrix<V, 3, 1>& v2, const Eigen::Matrix<V, 3, 1>& v3);

  template <class V>
  bool PtInTriangle(const Eigen::Matrix<V, 3, 1>& pt, const Eigen::Matrix<V, 3, 1>& v1, const Eigen::Matrix<V, 3, 1>& v2, const Eigen::Matrix<V, 3, 1>& v3);

  Eigen::Vector4f RegToAffine(const Eigen::Vector3f& v);
  Eigen::Vector3f AffineToReg(const Eigen::Vector4f& v);

  namespace debug {

    void DumpZBufferConsole(FrameBuffer& target);
    void DumpZBufferTGASimple(FrameBuffer& target);
    void DumpZBufferTGAFull(FrameBuffer& target);

  }
}
