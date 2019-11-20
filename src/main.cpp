#include <iostream>
#include <ostream>
#include <vector>
#include <array>
#include "../external/custom/tgaimage.hpp"
#include "../external/custom/model.hpp"
#include "render.hpp"

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const TGAColor green(0, 255, 0, 255);
const TGAColor blue(0, 0, 255, 255);

Eigen::Vector3i HeadAbsCoord(Eigen::Vector3f v, srd::FrameBuffer& frame) {
  return {
    static_cast<int>((v.x() + 1.0f) * frame.width() / 2.0f),
    static_cast<int>((v.y() + 1.0f) * frame.height() / 2.0f),
    static_cast<int>((v.z() + 1.0f) * frame.width() / 2.0f)
  };
}
void DrawDepthTestingDemo(srd::FrameBuffer& frame) {
  std::vector<Eigen::Vector3i> verts {
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
void DrawWireframeModel(srd::FrameBuffer& frame) {
  Model model("obj/african_head.obj");
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
    // 3 vertices per triangle
    for(int j = 0; j < 3; ++j) {
      // Absolute coordinates
      // +1 to move the normalized coordinate to 0 and 2 to make scaling easier
      // Scale the obj down by 1/2, because the input has a range of 2
      Eigen::Vector3i v1 = HeadAbsCoord(model.vert(face[j]), frame);
      Eigen::Vector3i v2 = HeadAbsCoord(model.vert(face[(j + 1) % 3]), frame);
      srd::DrawLine(v1, v2, frame, white);
    }
  }
}
void DrawRandomColorModel(srd::FrameBuffer& frame) {
  Model model("obj/african_head.obj");
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
      Eigen::Vector3i v1 = HeadAbsCoord(model.vert(face[0]), frame);
      Eigen::Vector3i v2 = HeadAbsCoord(model.vert(face[1]), frame);
      Eigen::Vector3i v3 = HeadAbsCoord(model.vert(face[2]), frame);
      TGAColor color(srd::RandInt(0, 256), srd::RandInt(0, 256), srd::RandInt(0, 256), 255);
      srd::DrawTriangle(v1, v2, v3, frame, color);
  }
}
void DrawSurfaceNormalColorModel(srd::FrameBuffer& frame, bool depthTest, bool correctGamma) {
  frame.SetDepthTest(depthTest);
  
  Eigen::Vector3f lightVec(0, 0, -1);
  Model model("obj/african_head.obj");
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
    std::array<Eigen::Vector3f, 3> wv;
    std::array<Eigen::Vector3i, 3> sv;
    std::cout << "Face: \n";
    for(int j = 0; j < 3; ++j) {
      wv[j] = model.vert(face[j]);
      sv[j] = HeadAbsCoord(wv[j], frame);
      std::cout << wv[j] << "\n\n";
    }

    Eigen::Vector3f normal = (wv[2] - wv[0]).cross(wv[1] - wv[0]).normalized();
    float lv = normal.dot(lightVec);
    if(lv > 0) {
      // No need to divide input color by 255 because it is normalized already
      int corrected = correctGamma
        ? 255 * pow(lv, 1 / 2.2)
        : 255 * lv;
      srd::DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
    }
  }
}

void DrawTeapot() {
  srd::FrameBuffer frame(800, 600);
  Model model("obj/teapot.obj");
  Eigen::Vector3f lightVec(0, 0, -1);
  for(int i = 0; i < model.nfaces(); ++i) {
    std::vector<int> face = model.face(i);
    std::array<Eigen::Vector3f, 3> wv;
    std::array<Eigen::Vector3i, 3> sv;
    for(int j = 0; j < 3; ++j) {
      wv[j] = model.vert(face[j]);
      sv[j] = {
        // Hardcoded translation & scaling
        static_cast<int>((wv[j].x() + 1.0f) * frame.width() / 8.0f) + 300,
        static_cast<int>((wv[j].y() + 1.0f) * frame.height() / 8.0f),
        static_cast<int>((wv[j].z() + 1.0f) * frame.width() / 8.0f)
      };
    }

    Eigen::Vector3f normal = (wv[2] - wv[0]).cross(wv[1] - wv[0]);
    normal.normalize();
    float lv = normal.dot(lightVec);
    if(lv > 0) {
      int corrected = 255 * pow(lv, 1 / 2.2);
      srd::DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
    }
  }
  frame.Write("./build/teapot.tga");
  srd::debug::DumpZBufferTGA(frame);
}

int main(int argc, char** argv) {
  std::cout << "Soft Renderer" << std::endl;

  // DrawTeapot();
  // return 0;

  srd::FrameBuffer frame(800, 800);
  // DrawDepthTestingDemo(frame);
  // Draw3DTrianglesTopDown(frame);
  // DrawWireframeModel(frame);
  // DrawRandomColorModel(frame);
  // DrawSurfaceNormalColorModel(frame, true, false);
  // DrawSurfaceNormalColorModel(frame, false, true);
  DrawSurfaceNormalColorModel(frame, true, true);
  frame.Write("./build/output.tga");
  srd::debug::DumpZBufferTGA(frame);

  return 0;
}
