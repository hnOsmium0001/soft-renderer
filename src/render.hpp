#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <Eigen/Dense>
#include "tgaimage.hpp"

namespace SRender {

  class FrameBuffer {
  private:
    TGAImage _image;
    std::vector<int> _zBuffer;

    bool enableDepthTest = true;
    bool enableDepthWrite = true;

  public:
    FrameBuffer(int width, int height, int defaultZ=0);
    void Write(const std::string path);
    int Test(int x, int y, int z);
    int Test(const Eigen::Vector3i& v);
    void Set(int x, int y, int z, TGAColor color);
    void Set(const Eigen::Vector3i& v, TGAColor color);
    void SetCb(int x, int y, int z, std::function<TGAColor()>&& cb);
    void SetCb(const Eigen::Vector3i& v, std::function<TGAColor()>&& cb);

    void SetDepthTest(bool enable) { enableDepthTest = enable; }
    void setDepthWrite(bool enable) { enableDepthWrite = enable; }

    // Getters
    int width() { return _image.get_width(); }
    int height() { return _image.get_width(); }
    TGAImage& image() { return _image; }
    const TGAImage& image() const { return _image; }
    std::vector<int>& zBuffer() { return _zBuffer; }
    const std::vector<int>& zBuffer() const { return _zBuffer; }
  };

  // Render pipline for 2D and 3D rendering

  class Pipeline {
  public:
    using VCallback = std::function<auto(const Eigen::Vector3f&) -> Eigen::Vector3i>;
    using FCallback = std::function<auto(const Eigen::Vector3i&, std::optional<const Eigen::Vector3f>) -> TGAColor>;
    using VerticesVector = const std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>>&;
    using RefVerticesVector = const std::vector<const Eigen::Vector3f&>;
    using IndicesVector = const std::vector<int>&;

  private:
    VCallback _vsh;
    FCallback _fsh;

  public:
    Pipeline() = default;
    void BindShader(VCallback vsh);
    void BindShader(FCallback fsh);
    void BindShaders(VCallback vsh, FCallback fsh);
    void DrawLines(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);
    void DrawTriangles(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);
    void DrawTriangleStrip(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);
    void DrawPolygon(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);

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

    Camera();
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
