#include "App.hpp"

#include "Color.hpp"
#include "Macros.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Scene.hpp"
#include "Viewer/Notification.hpp"
#include "Viewer/Utils.hpp"

#include <imgui.h>
#include <portable-file-dialogs.h>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <type_traits>

enum CurrentScene {
    ModelScene,
    TrianglesScene,
};

class ISceneData {
public:
    virtual bool IsReady() const = 0;
};

struct RenderData : public ISceneData {
    Camera camera;
    std::unique_ptr<Mesh> mesh;

    std::string meshFilePath;

    virtual bool IsReady() const override {
        return mesh != nullptr;
    }
};

struct TriangleScene {
    std::vector<glm::vec3> positions;
    std::vector<RgbaColor> colors;
};

struct TriangleSceneData : public ISceneData {
    TriangleScene scene;
    int currentSelectedVertex;

    virtual bool IsReady() const override {
        return true;
    }
};

struct App::Private {
    std::array<FrameBuffer, 2> canvases;
    Rasterizer rasterizer;
    Size2<int> canvasSize;
    GLuint texture;
    int displaying = 0;
    int drawing = 0 + 1;
    RgbaColor clearColor = RgbaColor(0, 0, 0);
    CurrentScene currScene = CurrentScene::ModelScene;

    RenderData rd;
    TriangleSceneData tsd;

    bool clearColorChanged = false;

    const ISceneData& GetCurrentScene() const {
        switch (currScene) {
            case CurrentScene::ModelScene: return rd;
            case CurrentScene::TrianglesScene: return tsd;
        }
        UNREACHABLE;
    }

    ISceneData& GetCurrentScene() {
        return const_cast<ISceneData&>(const_cast<const Private*>(this)->GetCurrentScene());
    }
};

App::App()
    : m{ new Private() } {
    constexpr int kWidth = 640;
    constexpr int kHeight = 320;

    constexpr int kSize = kWidth * kHeight;
    auto defaultTextureData = std::make_unique<RgbaColor[]>(kSize);
    std::fill(
        defaultTextureData.get(),
        defaultTextureData.get() + kSize,
        RgbaColor(255, 255, 255));

    ResizeCanvas({ kWidth, kHeight });
    glGenTextures(1, &m->texture);
    glBindTexture(GL_TEXTURE_2D, m->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultTextureData.get());
}

App::~App() {
    glDeleteTextures(1, &m->texture);

    delete m;
}

void App::Show() {
    ImGui::Begin("Renderer Setup");
    if (ImGui::BeginTabBar("##")) {
        if (ImGui::BeginTabItem("Render")) {
            ShowRendererEditor();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Model")) {
            ShowModelEditor();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Triangles Scene Setup")) {
            ShowTriangleEditor();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

    ImGui::Begin("Renderer Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Image(reinterpret_cast<void*>(m->texture), ImVec2(m->canvasSize.width, m->canvasSize.height));
    ImGui::End();

    ImGui::ShowNotifications();
}

void App::ShowRendererEditor() {
    constexpr EnumElement<CurrentScene> kScenes[] = {
        { "Model scene", CurrentScene::ModelScene },
        { "Triangles scene", CurrentScene::TrianglesScene },
    };
    if (ImGui::BeginCombo("Scene", kScenes[m->currScene].name)) {
        for (auto& elm : kScenes) {
            if (ImGui::Selectable(elm.name, m->currScene == elm.value)) {
                m->currScene = elm.value;
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::ColorEdit3("Clear color", &m->clearColor)) {
        m->clearColorChanged = true;
    }

    if (ImGui::TreeNode("Renderer Info")) {
        ImGui::Text("Buffers: %d", (int)m->canvases.size());
        ImGui::Indent();
        {
            ImGui::Text("Front buffer: #%d", m->displaying);
            ImGui::Text("Back buffer: #%d", m->drawing);
        }
        ImGui::Unindent();
        ImGui::Text("Canvas size: { %d, %d }", m->canvasSize.width, m->canvasSize.height);
        ImGui::TreePop();
    }

    auto& currScene = m->GetCurrentScene();

    if (ImGui::TreeNode("Scene Info")) {
        // TODO
        ImGui::TreePop();
    }

    bool readyToRender = currScene.IsReady();
    if (ImGui::Button("Render frame", !readyToRender)) {
        RenderFrame();
    }
    if (!readyToRender) {
        ImGui::SameLine();
        ImGui::TextUnformatted("The current scene is invalid");
    }
}

void App::ShowModelEditor() {
    if (ImGui::Button("Load mesh")) {
        pfd::open_file dialog("Select model file");
        auto result = dialog.result();

        if (!result.empty()) {
            auto& mesh = m->rd.mesh;
            auto& path = m->rd.meshFilePath;

            path = std::move(result[0]);
            if (!mesh) {
                mesh = std::make_unique<Mesh>();
            }

            bool exceptionCaught;
            try {
                mesh->ReadObjAt(path.c_str());
                exceptionCaught = false;
            } catch (const std::exception& e) {
                ImGui::AddNotification(ImGuiToast(ImGuiToastType_Error, "Failed to load model at %s.\nReason: %s", path.c_str(), e.what()));
                exceptionCaught = true;
            }
            if (!exceptionCaught) {
                ImGui::AddNotification(ImGuiToast(ImGuiToastType_Success, "Successfully loaded model at %s", path.c_str()));
            }
        } else {
            ImGui::AddNotification(ImGuiToast(ImGuiToastType_Error, "No path was selected."));
        }
    }
}

void App::ShowTriangleEditor() {
    if (m->tsd.currentSelectedVertex != -1) {
        // TODO
    } else {
        ImGui::TextUnformatted("Select an vertex to edit its properties");
    }

    ImGui::Begin("Triangles: scene editor");
    {
        auto& scene = m->tsd.scene;
        auto dl = ImGui::GetWindowDrawList();

        // Render all current triangles
        for (int i = 0, di = 0; i < scene.positions.size(); ++i, di += 3) {
            glm::vec3* triangle = &scene.positions[i];
            RgbaColor* colors = &scene.colors[di];

            // TODO handle multiple colors?
            // maybe use ImDrawList:: and vertices
            dl->AddTriangle(
                ImVec2(triangle[0].x, triangle[0].y),
                ImVec2(triangle[1].x, triangle[1].y),
                ImVec2(triangle[2].x, triangle[2].y),
                Conv::RgbaColor_To_ImU32(colors[0]));

            // TODO draw move-vertex handles
        }
    }
    ImGui::End();
}

static void UploadTexture(GLuint texture, const RgbaColor pixels[], Size2<int> size) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void App::RenderFrame() {
    auto& canvas = m->canvases[m->drawing];
    if (m->clearColorChanged) {
        canvas.ClearColor(m->clearColor);
    }

    switch (m->currScene) {
        case CurrentScene::ModelScene: {
            // TODO move rendering to other thread
            m->rasterizer.SetTarget(canvas);
            m->rasterizer.DrawMesh(m->rd.camera, *m->rd.mesh);
        } break;

        case CurrentScene::TrianglesScene: {
            m->rasterizer.SetTarget(canvas);
            for (int i = 0, di = 0; i < m->tsd.scene.positions.size(); ++i, di += 3) {
                auto positions = &m->tsd.scene.positions[i];
                auto colors = &m->tsd.scene.colors[di];
                m->rasterizer.DrawTriangle(positions, colors);
                i += 3;
            }
        } break;
    }
    ::UploadTexture(m->texture, canvas.pixels.data(), m->canvasSize);

    m->displaying = (m->displaying + 1) % m->canvases.size();
    m->drawing = (m->displaying + 1) % m->canvases.size();
}

void App::ResizeCanvas(Size2<int> newSize) {
    m->canvasSize = newSize;
    for (auto& canvas : m->canvases) {
        canvas.Resize(newSize);
    }
}
