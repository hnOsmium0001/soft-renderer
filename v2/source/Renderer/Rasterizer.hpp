#pragma once

#include "Color.hpp"
#include "Size.hpp"
#include "PodVector.hpp"

class FrameBuffer {
public:
    // Row-major
    PodVector<RgbaColor> pixels;
	Size2<int> dimensions;

public:
    FrameBuffer(Size2<int> dimensions);
};

class Rasterizer {
public:
    Rasterizer();

    // TODO draw triangle(s)
};
