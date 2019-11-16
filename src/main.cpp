#include <iostream>
#include <vector>
#include "../external/tgaimage.hpp"
#include "render.hpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

int main(int argc, char** argv) {
  std::cout << "Soft Renderer" << std::endl;
  
  srd::FrameBuffer frame(100, 100, -100);
  {
    std::vector<srd::Point> verts {
      {20, 10}, {40, 10}, {50, 30}, {30, 50}, {10, 30}
    };
    srd::DrawPolygon(verts, frame, red);
    
    srd::DrawTriangle({10, 10, -1}, {90, 10, -1}, {90, 90, -1}, frame, white);
  }
  frame.Write("./bin/output.tga");
  
  return 0;
}
