#pragma once

#include <vector>
#include "../external/tgaimage.hpp"
#include "../external/geometry.hpp"

// Stands for "Soft RenDerer"
namespace srd {

  class Camera {
  public:
    Vec3f _pos;
    Vec3f _angle;

    Camera(Vec3f pos = {0, 0, 0}, Vec3f angle = {0, 0, 0});
  };
  
  class FrameBuffer {
  private:
    Camera _cam;
    TGAImage _image;
    std::vector<int> _zBuffer;
    
    bool enableDepthTest = true;
    bool enableDepthWrite = true;
    
  public:
    FrameBuffer(int width, int height, int defaultZ=0);
    void Write(const std::string path);
    void Set(int x, int y, int z, TGAColor color);
    void Set(Vec3i v, TGAColor color);
    
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

  Vec3i& PtMin(Vec3i& p1, Vec3i& p2);
  Vec3i& PtMax(Vec3i& p1, Vec3i& p2);
  
  void DrawLine(Vec3i v1, Vec3i v2, FrameBuffer& frame, TGAColor color);
  void DrawTriangle(Vec3i v1, Vec3i v2, Vec3i v3, FrameBuffer& frame, TGAColor color);
  void DrawPolygon(std::vector<Vec3i> verts, FrameBuffer& frame, TGAColor color);
  
  Vec3f Barycentric(Vec3i pt, Vec3i v1, Vec3i v2, Vec3i v3);
  bool PtInTriangle(Vec3i pt, Vec3i v1, Vec3i v2, Vec3i v3);

  template <class N>
  N Map(N value, N fromMin, N fromMax, N toMin, N toMax);

  int RandIntRnage(int min, int max);

  namespace debug {
    
    void DumpZBufferConsole(FrameBuffer& target);
    void DumpZBufferTGA(FrameBuffer& target);

  }
}
