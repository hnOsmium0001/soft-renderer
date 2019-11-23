#include <iostream>
#include <ostream>
#include <vector>
#include <array>
#include <memory>
#include "../external/custom/tgaimage.hpp"
#include "../external/custom/model.hpp"
#include "render.hpp"

int RandInt(int min, int max) {
  static bool first = true;
  if (first) {
    std::srand(time(nullptr));
    first = false;
  }
  return min + rand() % ((max + 1) - min);
}

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const TGAColor green(0, 255, 0, 255);
const TGAColor blue(0, 0, 255, 255);

Eigen::Vector3i HeadAbsCoord(Eigen::Vector3f v, SRender::FrameBuffer& frame) {
  return {
    static_cast<int>((v.x() + 1.0f) * frame.width() / 2.0f),
    static_cast<int>((v.y() + 1.0f) * frame.height() / 2.0f),
    static_cast<int>((v.z() + 1.0f) * frame.width() / 2.0f)
  };
}
void DrawDepthTestingDemo(SRender::FrameBuffer& frame) {
  std::vector<Eigen::Vector3i> verts {
    {20, 10, 5}, {40, 10, 5}, {50, 30, 5}, {30, 50, 5}, {10, 30, 5}
  };
  SRender::DrawPolygon(verts, frame, red);
  SRender::DrawTriangle({10, 10, 1}, {90, 10, 1}, {90, 90, 1}, frame, white);
}
void Draw3DTrianglesTopDown(SRender::FrameBuffer& frame) {
  SRender::DrawTriangle({200, 120, 700}, {200, 680, 700}, {440, 400, 640}, frame, green);
  SRender::DrawTriangle({500, 40, 300}, {500, 760, 300}, {380, 400, 720}, frame, blue);
  SRender::DrawTriangle({700, 300, 660}, {700, 500, 660}, {100, 400, 40}, frame, red);
}
void DrawWireframeModel(SRender::FrameBuffer& frame) {
  auto model = std::make_unique<Model>("obj/african_head.obj");
  for(int i = 0; i < model->nfaces(); ++i) {
    auto face = model->face(i);
    // 3 vertices per triangle
    for(int j = 0; j < 3; ++j) {
      // Absolute coordinates
      // +1 to move the normalized coordinate to 0 and 2 to make scaling easier
      // Scale the obj down by 1/2, because the input has a range of 2
      auto v1 = HeadAbsCoord(model->vert(face[j]), frame);
      auto v2 = HeadAbsCoord(model->vert(face[(j + 1) % 3]), frame);
      SRender::DrawLine(v1, v2, frame, white);
    }
  }
}
void DrawRandomColorModel(SRender::FrameBuffer& frame) {
  auto model = std::make_unique<Model>("obj/african_head.obj");
  for(int i = 0; i < model->nfaces(); ++i) {
    auto face = model->face(i);
    auto v1 = HeadAbsCoord(model->vert(face[0]), frame);
    auto v2 = HeadAbsCoord(model->vert(face[1]), frame);
    auto v3 = HeadAbsCoord(model->vert(face[2]), frame);
    TGAColor color(RandInt(0, 256), RandInt(0, 256), RandInt(0, 256), 255);
    SRender::DrawTriangle(v1, v2, v3, frame, color);
  }
}
void DrawSurfaceNormalColorModel(SRender::FrameBuffer& frame, bool depthTest, bool correctGamma) {
  frame.SetDepthTest(depthTest);

  Eigen::Vector3f lightVec(0, 0, -1);
  auto model = std::make_unique<Model>("obj/african_head.obj");
  for(int i = 0; i < model->nfaces(); ++i) {
    auto face = model->face(i);
    std::array<Eigen::Vector3f, 3> wv;
    std::array<Eigen::Vector3i, 3> sv;
    for(int j = 0; j < 3; ++j) {
      wv[j] = model->vert(face[j]);
      sv[j] = HeadAbsCoord(wv[j], frame);
    }

    auto normal = (wv[2] - wv[0]).cross(wv[1] - wv[0]).normalized();
    float lv = normal.dot(lightVec);
    if(lv > 0) {
      // No need to divide input color by 255 because it is normalized already
      int corrected = correctGamma
        ? 255 * pow(lv, 1 / 2.2)
        : 255 * lv;
      SRender::DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
    }
  }
}

void DrawTeapot() {
  SRender::FrameBuffer frame(800, 600);
  Eigen::Vector3f lightVec(0, 0, -1);
  auto model = std::make_unique<Model>("obj/teapot.obj");
  for(int i = 0; i < model->nfaces(); ++i) {
    auto face = model->face(i);
    std::array<Eigen::Vector3f, 3> wv;
    std::array<Eigen::Vector3i, 3> sv;
    for(int j = 0; j < 3; ++j) {
      wv[j] = model->vert(face[j]);
      sv[j] = {
        // Hardcoded translation & scaling
        static_cast<int>((wv[j].x() + 1.0f) * frame.width() / 8.0f) + 300,
        static_cast<int>((wv[j].y() + 1.0f) * frame.height() / 8.0f),
        static_cast<int>((wv[j].z() + 1.0f) * frame.width() / 8.0f)
      };
    }

    auto normal = (wv[2] - wv[0]).cross(wv[1] - wv[0]).normalized();
    float lv = normal.dot(lightVec);
    if(lv > 0) {
      int corrected = 255 * pow(lv, 1 / 2.2);
      SRender::DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
    }
  }
  frame.Write("./build/teapot.tga");
  SRender::debug::DumpZBufferTGAFull(frame);
}

void DrawCameraHeadModel(SRender::FrameBuffer& frame) {
  SRender::Camera cam {};
  cam.LookAt({0, 0, 3}, {0, 1, 0}, {0, 0, 0});
  cam.Viewport(frame.width() / 8, frame.height() / 8, frame.width() * 3/4, frame.height() * 3/4);

  // Environment
  Eigen::Vector3f lightVec(0, 0, -1);

  auto model = std::make_unique<Model>("obj/african_head.obj");
  for(int i = 0; i < model->nfaces(); ++i) {
    auto face = model->face(i);
    std::array<Eigen::Vector3f, 3> wv;
    std::array<Eigen::Vector3i, 3> sv;
    for(int j = 0; j < 3; ++j) {
      wv[j] = model->vert(face[j]);
      sv[j] = cam.ToScreen(wv[j]);
    }

    auto normal = (wv[2] - wv[0]).cross(wv[1] - wv[0]).normalized();
    float lv = normal.dot(lightVec);
    if(lv > 0) {
      int corrected = 255 * pow(lv, 1 / 2.2);
      SRender::DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
    }
  }
}

int main(int argc, char** argv) {
  std::cout << "Soft Renderer" << std::endl;

  // DrawTeapot();
  // return 0;

  SRender::FrameBuffer frame(800, 800);
  // DrawDepthTestingDemo(frame);
  // Draw3DTrianglesTopDown(frame);
  // DrawWireframeModel(frame);
  // DrawRandomColorModel(frame);
  // DrawSurfaceNormalColorModel(frame, true, false);
  // DrawSurfaceNormalColorModel(frame, false, true);
  // DrawSurfaceNormalColorModel(frame, true, true);
  DrawCameraHeadModel(frame);
  frame.Write("./build/output.tga");
  SRender::debug::DumpZBufferTGAFull(frame);

  return 0;
}
