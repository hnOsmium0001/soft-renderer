#include <algorithm>
#include "tgaimage.hpp"
#include "render.hpp"

srd::Point::Point(int x, int y)
: x{x}, y{y} {
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

void srd::DrawLine(srd::Point v1, srd::Point v2, TGAImage& image, TGAColor color) {
  srd::Point p1 {std::min(v1.x, v2.x), std::min(v1.y, v2.y)};
  srd::Point p2 {std::max(v2.x, v2.x), std::max(v1.y, v2.y)};
  
  if(p2.y - p1.y > p2.x - p1.x) {
    // Main axis: Y
    for(int y = p1.y; y <= p2.y; ++y) {
      float t = y / static_cast<float>(p2.y - p1.y);
      int x = p1.x + (p2.x - p1.x) * t;
      image.set(x, y, color);
    }
  } else {
    // Main axis: X
    for(int x = p1.x; x <= p2.x; ++x) {
      float t = x / static_cast<float>(p2.x - p1.x);
      int y = p1.y + (p2.y - p1.y) * t;
      image.set(x, y, color);
    }
  }
}
void srd::DrawTriangle(srd::Point v1, srd::Point v2, srd::Point v3, TGAImage& image, TGAColor color) {
  // Stands for "Boudning Box Vertex N"
  srd::Point bbv1 {std::min({v1.x, v2.x, v3.x}), std::min({v1.y, v2.y, v3.x})};
  srd::Point bbv2 {std::max({v1.x, v2.x, v3.x}), std::max({v1.y, v2.y, v3.x})};
  for(int y = bbv1.y; y <= bbv2.y; ++y) {
    for(int x = bbv1.x; x <= bbv2.x; ++x) {
      if(srd::PtInTriangle({x, y}, v1, v2, v3)) {
        image.set(x, y, color);
      }
    }
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
