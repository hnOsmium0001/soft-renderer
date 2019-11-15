#pragma once

#include "tgaimage.hpp"

// Stands for "Soft RenDerer"
namespace srd {
  
  class Point {
  public:
    const int x;
    const int y;
    
    Point(int x, int y);
  };
  
  Point& PtMin(Point& p1, Point& p2);
  Point& PtMax(Point& p1, Point& p2);
  
  void DrawLine(Point v1, Point v2, TGAImage& image, TGAColor color);
  void DrawTriangle(Point v1, Point v2, Point v3, TGAImage& image, TGAColor color);
  
  bool PtInTriangle(Point pt, Point v1, Point v2, Point v3);
  
//   class Line {
//   public:
//     const Point p1;
//     const Point p2;
//     
//     Line(Point p1, Point p2);
//     void Draw(TGAImage& image, TGAColor color) const;
//   };
  
}
