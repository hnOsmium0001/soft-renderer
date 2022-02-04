#pragma once

#include "Color.hpp"
#include "Renderer/Rasterizer.hpp"
#include "Renderer/Scene.hpp"
#include "Size.hpp"

#include <glad/glad.h>
#include <array>
#include <memory>
#include <vector>

struct TriangleScene {
    std::vector<Triangle> triangles;
    std::vector<RgbaColor> triangleColors;
};

class App {
private:
    std::array<FrameBuffer, 2> mCanvases;
    Camera mCamera;
    Rasterizer mRasterizer;
    Size2<int> mCanvasSize;
    GLuint mTexture;
    int mDisplaying = 0;
    int mDrawing = 0 + 1;

    std::unique_ptr<Mesh> mesh;

    TriangleScene scene;

public:
    App();
    ~App();

    void Show();
    void ShowRendererEditor();
    void ShowTriangleEditor();

    void RenderFrame();
    void ResizeCanvas(Size2<int> newSize);
};