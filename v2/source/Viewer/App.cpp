#include "App.hpp"

#include "Color.hpp"

#include <imgui.h>
#include <algorithm>
#include <memory>

App::App() {
    constexpr int kWidth = 640;
    constexpr int kHeight = 320;

    constexpr int kSize = kWidth * kHeight;
    auto defaultTextureData = std::make_unique<RgbaColor[]>(kSize);
    std::fill(
        defaultTextureData.get(),
        defaultTextureData.get() + kSize,
        RgbaColor(255, 255, 255));

    ResizeCanvas({ kWidth, kHeight });
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultTextureData.get());
}

App::~App() {
    glDeleteTextures(1, &mTexture);
}

void App::Show() {
    ImGui::Begin("Renderer Setup");
    if (ImGui::BeginTabBar("##")) {
        if (ImGui::BeginTabItem("Render")) {
            ShowRendererEditor();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Triangles")) {
            ShowTriangleEditor();
            ImGui::EndTabItem();
        }
    }
    ImGui::End();

    ImGui::Begin("Renderer Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Image(reinterpret_cast<void*>(mTexture), ImVec2(mCanvasSize.width, mCanvasSize.height));
    ImGui::End();
}

void App::ShowRendererEditor() {
    if (ImGui::TreeNode("Info")) {
        ImGui::Text("Buffers: %d", (int)mCanvases.size());
        ImGui::Indent();
        {
            ImGui::Text("Front buffer: #%d", mDisplaying);
            ImGui::Text("Back buffer: #%d", mDrawing);
        }
        ImGui::Unindent();
        ImGui::Text("Canvas size: { %d, %d }", mCanvasSize.width, mCanvasSize.height);
        ImGui::TreePop();
    }
    if (ImGui::Button("Render frame")) {
        RenderFrame();
    }
}

void App::ShowTriangleEditor() {
    // TODO
}

static void UploadTexture(GLuint texture, const RgbaColor pixels[], Size2<int> size) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void App::RenderFrame() {
    // TODO move rendering to other thread
#if 0
    mRasterizer.SetTarget(mCanvases[mDrawing]);
    mRasterizer.DrawMesh(mCamera, *mesh);
#else
    mRasterizer.SetTarget(mCanvases[mDrawing]);
    int i = 0;
    for (auto& tri : scene.triangles) {
        mRasterizer.DrawTriangle(tri.vertices, &scene.triangleColors[i]);
        i += 3;
    }
#endif
    ::UploadTexture(mTexture, mCanvases[mDrawing].pixels.data(), mCanvasSize);

    mDisplaying = (mDisplaying + 1) % mCanvases.size();
    mDrawing = (mDisplaying + 1) % mCanvases.size();
}

void App::ResizeCanvas(Size2<int> newSize) {
    mCanvasSize = newSize;
    for (auto& canvas : mCanvases) {
        canvas.Resize(newSize);
    }
}
