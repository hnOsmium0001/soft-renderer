#pragma once

#include <vector>
#include "../external/tgaimage.hpp"

// Stands for "Soft RenDerer"
namespace srd {
  
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
  
  class Point {
  public:
    int x;
    int y;
    int z;
    
    Point(int x, int y, int z=0);
  };
  
  Point& PtMin(Point& p1, Point& p2);
  Point& PtMax(Point& p1, Point& p2);
  
  void DrawLine(Point v1, Point v2, FrameBuffer& frame, TGAColor color);
  void DrawRect(Point v1, Point v2, FrameBuffer& frame, TGAColor color);
  void DrawTriangle(Point v1, Point v2, Point v3, FrameBuffer& frame, TGAColor color);
  void DrawPolygon(std::vector<Point> verts, FrameBuffer& frame, TGAColor color);
  
  bool PtInTriangle(Point pt, Point v1, Point v2, Point v3);
}
