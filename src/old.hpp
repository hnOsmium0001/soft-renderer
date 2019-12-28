#include <vector>
#include "../external/custom/tgaimage.hpp"
#include "render.hpp"

void DrawLine(
  const Eigen::Vector3i& v1In,
  const Eigen::Vector3i& v2In,
  SRender::FrameBuffer& frame,
  TGAColor color
);

void DrawTriangle(
  const Eigen::Vector3i& v1,
  const Eigen::Vector3i& v2,
  const Eigen::Vector3i& v3,
  SRender::FrameBuffer& frame,
  TGAColor color
);

void DrawPolygon(
  const std::vector<Eigen::Vector3i>& verts,
  SRender::FrameBuffer& frame,
  TGAColor color
);
