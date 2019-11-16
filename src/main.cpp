#include <iostream>
#include <ostream>
#include <vector>
#include "../external/tgaimage.hpp"
#include "../external/geometry.hpp"
#include "../external/model.hpp"
#include "render.hpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void DrawDepthTestingDemo(srd::FrameBuffer& frame) {
  std::vector<Vec3i> verts {
    {20, 10, 0}, {40, 10, 0}, {50, 30, 0}, {30, 50, 0}, {10, 30, 0}
  };
  srd::DrawPolygon(verts, frame, red);
  srd::DrawTriangle({10, 10, -1}, {90, 10, -1}, {90, 90, -1}, frame, white);
}
void DrawWireframeModel(srd::FrameBuffer& frame) {
  Model model("obj/african_head.obj");
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
    // 3 vertices per triangle
    for(int j = 0; j < 3; ++j) {
      // Normalized coordinates between -1 and 1
      Vec3f sv1 = model.vert(face[j]);
      Vec3f sv2 = model.vert(face[j + 1 >= 3 ? 0 : j + 1]);
      // Absolute coordinates
      // +1 to move the normalized coordinate to 0 and 2 to make scaling easier
      // Scale the obj down by 1/2, because the input has a range of 2
      Vec3i v1 {
        static_cast<int>((sv1.x + 1) * frame.width() / 2),
        static_cast<int>((sv1.y + 1) * frame.height() / 2),
        0
      };
      Vec3i v2 {
        static_cast<int>((sv2.x + 1) * frame.width() / 2),
        static_cast<int>((sv2.y + 1) * frame.height() / 2),
        0
      };

      srd::DrawLine(v1, v2, frame, white);
    }
  }
}

int main(int argc, char** argv) {
  std::cout << "Soft Renderer" << std::endl;
  
  srd::FrameBuffer frame(800, 800, -255);
//   DrawDepthTestingDemo(frame);
  DrawWireframeModel(frame);
  frame.Write("./bin/output.tga");
  
  return 0;
}
