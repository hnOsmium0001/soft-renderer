#include <vector>
#include <algorithm>
#include "../external/tgaimage.hpp"
#include "render.hpp"

srd::FrameBuffer::FrameBuffer(int width, int height, int defaultZ)
: _image{width, height, TGAImage::RGB}, _zBuffer(width * height, defaultZ) {
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

srd::Point::Point(int x, int y, int z)
: x{x}, y{y}, z{z} {
}

srd::Point& srd::PtMin(srd::Point& p1, srd::Point& p2) {
  if(p1.x > p2.x) return p2;
  if(p1.y > p2.y) return p2;
  return p1;
}
srd::Point& srd::PtMax(srd::Point& p1, srd::Point& p2) {
  if(p1.x < p2.x) return p2;
  if(p1.y < p2.y) return p2;
  return p1;
}

void srd::DrawLine(srd::Point v1, srd::Point v2, FrameBuffer& frame, TGAColor color) {
  bool steep = false;
  if (std::abs(v1.x - v2.x) < std::abs(v1.y - v2.y)) {
    std::swap(v1.x, v1.y); 
    std::swap(v2.x, v2.y); 
    steep = true; 
  }
  if(v1.x > v2.x) {
    std::swap(v1, v2);
  }
  for(int x = v1.x; x <= v2.x; ++x) {
    float t = (x - v1.x) / static_cast<float>(v2.x - v1.x);
    int y = v1.y * (1.0 - t) + v2.y * t;
    int z = v1.z * (1.0 - t) + v2.z * t;
    if(steep) {
      frame.Set(y ,x, z, color);
    } else {
      frame.Set(x, y, z, color);
    }
  }
}
void srd::DrawRect(srd::Point v1, srd::Point v2, FrameBuffer& frame, TGAColor color) {
  srd::DrawLine(v1, {v2.x, v1.y}, frame, color);
  srd::DrawLine({v2.x, v1.y}, v2, frame, color);
  srd::DrawLine(v2, {v1.x, v2.y}, frame, color);
  srd::DrawLine({v1.x, v2.y}, v1, frame, color);
}
void srd::DrawTriangle(srd::Point v1, srd::Point v2, srd::Point v3, FrameBuffer& frame, TGAColor color) {
  // Stands for "Boudning Box Vertex N"
  srd::Point bbv1 {std::min({v1.x, v2.x, v3.x}), std::min({v1.y, v2.y, v3.y})};
  srd::Point bbv2 {std::max({v1.x, v2.x, v3.x}), std::max({v1.y, v2.y, v3.y})};
  
  for(int y = bbv1.y; y <= bbv2.y; ++y) {
    for(int x = bbv1.x; x <= bbv2.x; ++x) {
      if(srd::PtInTriangle({x, y}, v1, v2, v3)) {
        frame.Set(x, y, v1.z, color);
      }
    }
  }
}
void srd::DrawPolygon(std::vector<Point> verts, FrameBuffer& frame, TGAColor color) {
  if(verts.size() < 3) return;
  
  auto lastOne {verts[1]};
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    srd::DrawTriangle(verts[0], lastOne, *it, frame, color);
    lastOne = *it;
  }
}


float Sign(srd::Point p1, srd::Point p2, srd::Point p3) {
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}
bool srd::PtInTriangle(srd::Point pt, srd::Point v1, srd::Point v2, srd::Point v3) {
  float d1 = Sign(pt, v1, v2);
  float d2 = Sign(pt, v2, v3);
  float d3 = Sign(pt, v3, v1);
  bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
  return !(hasNeg && hasPos);
}
