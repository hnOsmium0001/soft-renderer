#pragma once

#include "Color.hpp"
#include "Renderer/Rasterizer.hpp"
#include "Size.hpp"

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

    void RenderFrame();
};
