#pragma once

#include "Color.hpp"
#include "Renderer/Rasterizer.hpp"
#include "Size.hpp"

#include <glad/glad.h>
#include <array>
#include <vector>

class App {
private:
    struct Private;
    Private* m;

public:
    App();
    ~App();

    void Show();
    void ShowRendererEditor();
    void ShowModelEditor();
    void ShowTriangleEditor();

    void RenderFrame();
    void ResizeCanvas(Size2<int> newSize);
};