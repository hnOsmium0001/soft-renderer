#pragma once

#include <vector>
#include "../external/Eigen/Dense"
#include "../external/custom/tgaimage.hpp"

// Stands for "Soft RenDerer"
namespace srd {

  class Camera {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Eigen::Vector3f _pos;
    Eigen::Vector3f _angle;

    Camera(Eigen::Vector3f pos = {0, 0, 0}, Eigen::Vector3f angle = {0, 0, 0});
  };

  class FrameBuffer {
  private:
    Camera _cam;
    TGAImage _image;
    std::vector<int> _zBuffer;

    bool enableDepthTest = true;
    bool enableDepthWrite = true;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

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

  const Eigen::Vector3i& PtMin(const Eigen::Vector3i& p1, const Eigen::Vector3i& p2);
  const Eigen::Vector3i& PtMax(const Eigen::Vector3i& p1, const Eigen::Vector3i& p2);

  void DrawLine(const Eigen::Vector3i& v1, const Eigen::Vector3i& v2, FrameBuffer& frame, TGAColor color);
  void DrawTriangle(const Eigen::Vector3i& v1, const Eigen::Vector3i& v2, const Eigen::Vector3i& v3, FrameBuffer& frame, TGAColor color);
  void DrawTriangles(const std::vector<Eigen::Vector3i>& verts, const std::vector<int> indices, FrameBuffer& frame, TGAColor color);
  void DrawTriangleStrip(const std::vector<Eigen::Vector3i>& verts, FrameBuffer& frame, TGAColor color);
  void DrawPolygon(const std::vector<Eigen::Vector3i>& verts, FrameBuffer& frame, TGAColor color);

  template <class N>
  N Map(N value, N fromMin, N fromMax, N toMin, N toMax);

  Eigen::Vector3f Barycentric(const Eigen::Vector3i& pt, const Eigen::Vector3i& v1, const Eigen::Vector3i& v2, const Eigen::Vector3i& v3);
  bool PtInTriangle(const Eigen::Vector3i& pt, const Eigen::Vector3i& v1, const Eigen::Vector3i& v2, const Eigen::Vector3i& v3);

  namespace debug {

    void DumpZBufferConsole(FrameBuffer& target);
    void DumpZBufferTGA(FrameBuffer& target);

  }
}
