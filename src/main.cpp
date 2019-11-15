#include <iostream>
#include "tgaimage.hpp"

const uint32_t width = 100;
const uint32_t height = 100;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);

void draw(TGAImage& image) {
  
}

int main(int argc, char** argv) {
  std::cout << "Soft Renderer" << std::endl;
  
  TGAImage image(width, height, TGAImage::RGB);
  image.set(52, 41, red);
  image.flip_vertically();
  draw(image);
  image.write_tga_file("output.tga");
  
  return 0;
}
