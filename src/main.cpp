#include <iostream>
#include <ostream>
#include <vector>
#include "../external/tgaimage.hpp"
#include "../external/geometry.hpp"
#include "../external/model.hpp"
#include "render.hpp"

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const TGAColor green(0, 255, 0, 255);
const TGAColor blue(0, 0, 255, 255);

void DrawDepthTestingDemo(srd::FrameBuffer& frame) {
  std::vector<Vec3i> verts {
    {20, 10, 5}, {40, 10, 5}, {50, 30, 5}, {30, 50, 5}, {10, 30, 5}
  };
  srd::DrawPolygon(verts, frame, red);
  srd::DrawTriangle({10, 10, 1}, {90, 10, 1}, {90, 90, 1}, frame, white);
}
void Draw3DTrianglesTopDown(srd::FrameBuffer& frame) {
  srd::DrawTriangle({200, 120, 700}, {200, 680, 700}, {440, 400, 640}, frame, green);
  srd::DrawTriangle({500, 40, 300}, {500, 760, 300}, {380, 400, 720}, frame, blue);
  srd::DrawTriangle({700, 300, 660}, {700, 500, 660}, {100, 400, 40}, frame, red);
}

Vec3i CalcAbsCoord(Vec3f v, srd::FrameBuffer& frame) {
  return {
    static_cast<int>((v.x + 1) * frame.width() / 2),
    static_cast<int>((v.y + 1) * frame.height() / 2),
    static_cast<int>((v.z + 1) * frame.width() / 2)
  };
}
void DrawWireframeModel(srd::FrameBuffer& frame) {
  Model model("obj/african_head.obj");
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
    // 3 vertices per triangle
    for(int j = 0; j < 3; ++j) {
      // Absolute coordinates
      // +1 to move the normalized coordinate to 0 and 2 to make scaling easier
      // Scale the obj down by 1/2, because the input has a range of 2
      Vec3i v1 = CalcAbsCoord(model.vert(face[j]), frame);
      Vec3i v2 = CalcAbsCoord(model.vert(face[j + 1 >= 3 ? 0 : j + 1]), frame);
      srd::DrawLine(v1, v2, frame, white);
    }
  }
}
void DrawRandomColorModel(srd::FrameBuffer& frame) {
  Model model("obj/african_head.obj");
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
      Vec3i v1 = CalcAbsCoord(model.vert(face[0]), frame);
      Vec3i v2 = CalcAbsCoord(model.vert(face[1]), frame);
      Vec3i v3 = CalcAbsCoord(model.vert(face[2]), frame);
      TGAColor color(srd::RandIntRnage(0, 256), srd::RandIntRnage(0, 256), srd::RandIntRnage(0, 256), 255);
      srd::DrawTriangle(v1, v2, v3, frame, color);
  }
}
void DrawSurfaceNormalColorModel(srd::FrameBuffer& frame, bool depthTest, bool correctGamma) {
  frame.SetDepthTest(depthTest);
  
  Vec3f lightVec(0, 0, -1);
  Model model("obj/african_head.obj");
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
    Vec3f wv[3];
    Vec3i sv[3];
    for(int j = 0; j < 3; ++j) {
      wv[j] = model.vert(face[j]);
      sv[j] = CalcAbsCoord(wv[j], frame);
    }

    Vec3f normal = (wv[2] - wv[0]) ^ (wv[1] - wv[0]);
    normal.normalize();
    float lv = normal * lightVec;
    if(lv > 0) {
      // No need to divide input color by 255 because it is normalized already
      int corrected = correctGamma
        ? 255 * pow(lv, 1 / 2.2)
        : 255 * lv;
      srd::DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
    }
  }
}

int main(int argc, char** argv) {
  std::cout << "Soft Renderer" << std::endl;
  srd::Camera cam;

  srd::FrameBuffer frame(800, 800);
  // DrawDepthTestingDemo(frame);
  // Draw3DTrianglesTopDown(frame);
  // DrawWireframeModel(frame);
  // DrawRandomColorModel(frame);
  // DrawSurfaceNormalColorModel(frame, true, false);
  // DrawSurfaceNormalColorModel(frame, false, true);
  DrawSurfaceNormalColorModel(frame, true, true);
  frame.Write("./bin/output.tga");

  srd::debug::DumpZBufferTGA(frame);

  return 0;
}
