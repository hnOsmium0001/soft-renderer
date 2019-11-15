#include <iostream>
#include "tgaimage.hpp"
#include "render.hpp"

const int width = 100;
const int height = 100;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void draw(TGAImage& image) {
//   srd::DrawLine({13, 20}, {80, 40}, image, white);
//   srd::DrawLine({20, 13}, {40, 80}, image, red);
//   srd::DrawLine({40, 60}, {10, 20}, image, red);
  srd::DrawTriangle({10, 10}, {10, 60}, {80, 60}, image, red);
  srd::DrawTriangle({10, 10}, {80, 10}, {80, 60}, image, red);
}

int main(int argc, char** argv) {
  std::cout << "Soft Renderer" << std::endl;
  
  TGAImage image(width, height, TGAImage::RGB);
  draw(image);
  image.flip_vertically();
  image.write_tga_file("./bin/output.tga");
  
  return 0;
}
