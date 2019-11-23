#pragma once

#include <vector>
#include "../external/Eigen/Dense"
#include "../external/custom/tgaimage.hpp"

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

  // 3D rendering

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

    Eigen::Vector3i ToScreen(const Eigen::Vector3f& pt);
  };

  // Misc utilities

  template <class N>
  N Map(N value, N fromMin, N fromMax, N toMin, N toMax);

  template <class V>
  Eigen::Matrix<V, 3, 1> Barycentric(const Eigen::Matrix<V, 3, 1>& pt, const Eigen::Matrix<V, 3, 1>& v1, const Eigen::Matrix<V, 3, 1>& v2, const Eigen::Matrix<V, 3, 1>& v3);

  template <class V>
  bool PtInTriangle(const Eigen::Matrix<V, 3, 1>& pt, const Eigen::Matrix<V, 3, 1>& v1, const Eigen::Matrix<V, 3, 1>& v2, const Eigen::Matrix<V, 3, 1>& v3);

  namespace debug {

    void DumpZBufferConsole(FrameBuffer& target);
    void DumpZBufferTGASimple(FrameBuffer& target);
    void DumpZBufferTGAFull(FrameBuffer& target);

  }
}
