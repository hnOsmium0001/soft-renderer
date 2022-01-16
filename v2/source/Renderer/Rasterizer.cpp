#include "Rasterizer.hpp"

#include "Color.hpp"

FrameBuffer::FrameBuffer(Size2<int> dimensions)
    : dimensions{ dimensions } {
    pixels.resize(dimensions.Area(), RgbaColor(0, 0, 0));
}

Rasterizer::Rasterizer() {
}
