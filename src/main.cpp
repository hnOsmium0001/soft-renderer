#include <iostream>
#include <ostream>
#include <vector>
#include <array>
#include <memory>
#include "../external/custom/tgaimage.hpp"
#include "../external/custom/model.hpp"
#include "render.hpp"

// ====================================
// Drawing 2D primitives
// ====================================

void DrawLine(
  const Eigen::Vector3i& v1In,
  const Eigen::Vector3i& v2In,
  SRender::FrameBuffer& frame,
  TGAColor color
) {
  auto v1 = v1In;
  auto v2 = v2In;

  // Swapping is safe because pass-by-value
  bool steep = false;
  if (std::abs(v1.x() - v2.x()) < std::abs(v1.y() - v2.y())) {
    std::swap(v1.x(), v1.y());
    std::swap(v2.x(), v2.y());
    steep = true;
  }
  if(v1.x() > v2.x()) {
    std::swap(v1, v2);
  }
  for(int x = v1.x(); x <= v2.x(); ++x) {
    float t = (x - v1.x()) / static_cast<float>(v2.x() - v1.x());
    int y = v1.y() * (1.0 - t) + v2.y() * t;
    int z = v1.z() * (1.0 - t) + v2.z() * t;
    if(steep) {
      frame.Set(y ,x, z, color);
    } else {
      frame.Set(x, y, z, color);
    }
  }
}

void DrawTriangle(
  const Eigen::Vector3i& v1,
  const Eigen::Vector3i& v2,
  const Eigen::Vector3i& v3,
  SRender::FrameBuffer& frame,
  TGAColor color
) {
  // Stands for "Boudning Box Vertex 1/2"
  Eigen::Vector2i bbv1 {
    std::max(0, std::min({v1.x(), v2.x(), v3.x()})),
    std::max(0, std::min({v1.y(), v2.y(), v3.y()}))
  };
  Eigen::Vector2i bbv2 {
    std::min(frame.width(), std::max({v1.x(), v2.x(), v3.x()})),
    std::min(frame.height(), std::max({v1.y(), v2.y(), v3.y()}))
  };

  for(int y = bbv1.y(); y <= bbv2.y(); ++y) {
    for(int x = bbv1.x(); x <= bbv2.x(); ++x) {
      auto bc = SRender::Barycentric({x, y, 0}, v1, v2, v3);
      if(SRender::PtInTriangle({x, y, 0}, v1, v2, v3)) {
        float z =
          v1.z() * bc.x() +
          v2.z() * bc.y() +
          v3.z() * bc.z();
        frame.Set(x, y, z, color);
      }
    }
  }
}

void DrawTriangles(
  const std::vector<Eigen::Vector3i> &verts,
  const std::vector<int> indices,
  SRender::FrameBuffer &frame,
  TGAColor color
) {
  assert(verts.size() >= 3);
  assert(indices.size() % 3 == 0);

  for(int i = 0; i < indices.size(); i += 3) {
    DrawTriangle(verts[indices[i]], verts[indices[i + 1]], verts[indices[i + 2]], frame, color);
  }
}

void DrawPolygon(
  const std::vector<Eigen::Vector3i>& verts,
  SRender::FrameBuffer& frame,
  TGAColor color
) {
  assert(verts.size() >= 3);

  auto lastOne = &verts[1];
  for(auto it = std::next(verts.begin(), 2); it != verts.end(); ++it) {
    DrawTriangle(verts[0], *lastOne, *it, frame, color);
    lastOne = &*it;
  }
}

// ====================================
// Render application code
// ====================================

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
  DrawPolygon(verts, frame, red);
  DrawTriangle({10, 10, 1}, {90, 10, 1}, {90, 90, 1}, frame, white);
}
void Draw3DTrianglesTopDown(SRender::FrameBuffer& frame) {
  DrawTriangle({200, 120, 700}, {200, 680, 700}, {440, 400, 640}, frame, green);
  DrawTriangle({500, 40, 300}, {500, 760, 300}, {380, 400, 720}, frame, blue);
  DrawTriangle({700, 300, 660}, {700, 500, 660}, {100, 400, 40}, frame, red);
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
      DrawLine(v1, v2, frame, white);
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
    DrawTriangle(v1, v2, v3, frame, color);
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
      DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
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

    Eigen::Vector3f normal = (wv[2] - wv[0]).cross(wv[1] - wv[0]).normalized();
    float lv = normal.dot(lightVec);
    if(lv > 0) {
      int corrected = 255 * pow(lv, 1 / 2.2);
      DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
    }
  }
  frame.Write("./build/teapot.tga");
  SRender::debug::DumpZBufferTGAFull(frame);
}

void DrawCameraHeadModel(SRender::FrameBuffer& frame) {
  Eigen::Vector3f camPos;
  std::cout << "Enter camera position in order of x, y, z: \n";
  std::cin >> camPos.x() >> camPos.y() >> camPos.z();

  SRender::Camera cam {};
  cam.LookAt(camPos, {0, 1, 0}, {0, 0, 0});
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
      sv[j] = cam.Transform(wv[j]).cast<int>();
    }

    Eigen::Vector3f normal = (wv[2] - wv[0]).cross(wv[1] - wv[0]).normalized();
    float lv = normal.dot(lightVec);
    if(lv > 0) {
      int corrected = 255 * pow(lv, 1 / 2.2);
      DrawTriangle(sv[0], sv[1], sv[2], frame, TGAColor(corrected, corrected, corrected, 255));
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
