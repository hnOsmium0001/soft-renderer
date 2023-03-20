#include "App.hpp"

#include "Color.hpp"
#include "Macros.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Scene.hpp"
#include "Viewer/Notification.hpp"
#include "Viewer/Utils.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <portable-file-dialogs.h>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace {
enum class SceneType {
    Model,
    Triangles,
};

class ISceneData {
public:
    virtual bool IsReady() const = 0;
};

struct ModelSceneData : public ISceneData {
    RgbaColor clearColor = RgbaColor(255, 255, 255);
    Camera camera;
    std::unique_ptr<Mesh> mesh;

    std::string meshFilePath;
    float clearDepth = 0.0f;

    virtual bool IsReady() const override {
        return mesh != nullptr;
    }
};

struct TriangleSceneData : public ISceneData {
    RgbaColor clearColor = RgbaColor(255, 255, 255);
    std::vector<glm::vec3> positions;
    std::vector<RgbaColor> colors;
    int currentSelectedVertex;
    float clearDepth = 0.0f;

    virtual bool IsReady() const override {
        return true;
    }
};

void UploadTexture(GLuint texture, const RgbaColor pixels[], Size2<int> size) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
} // namespace

struct App::Private {
    FrameBuffer canvas;
    Rasterizer rasterizer;
    Size2<int> canvasSize;
    GLuint texture = 0;

    SceneType currSceneType = SceneType::Model;
    ModelSceneData rd;
    TriangleSceneData tsd;

    Private() {
        // Goal of this constructor:
        // Setup any necessary resources/allocations for the App to function, any default values should be initialized in App::App()

        rasterizer.SetTarget(&canvas);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Even though most drivers permit it, using a texture without initializing it with an image is undefined so we do it here
        UploadBuffers();
    }

    ~Private() {
        glDeleteTextures(1, &texture);
    }

    const ISceneData& GetCurrentScene() const {
        switch (currSceneType) {
            case SceneType::Model: return rd;
            case SceneType::Triangles: return tsd;
        }
        UNREACHABLE;
    }

    ISceneData& GetCurrentScene() {
        return const_cast<ISceneData&>(const_cast<const Private*>(this)->GetCurrentScene());
    }

    void RenderCurrentScene() {
        switch (currSceneType) {
            case SceneType::Model: {
                // TODO move rendering to other thread
                canvas.ClearColor(rd.clearColor);
                canvas.ClearDepth(rd.clearDepth);

                rasterizer.DrawMesh(rd.camera, *rd.mesh);
            } break;

            case SceneType::Triangles: {
                canvas.ClearColor(tsd.clearColor);
                canvas.ClearDepth(tsd.clearDepth);

                for (int i = 0, di = 0; i < tsd.positions.size(); ++i, di += 3) {
                    auto positions = &tsd.positions[i];
                    auto colors = &tsd.colors[di];
                    rasterizer.DrawTriangle(positions, colors);
                    i += 3;
                }
            } break;
        }
    }

    void ResizeCanvas(Size2<int> newSize) {
        canvasSize = newSize;
        canvas.Resize(newSize);
    }

    void UploadBuffers() {
        ::UploadTexture(texture, canvas.pixels.data(), canvasSize);
    }

    void ShowRendererEditor() {
        constexpr EnumElement<SceneType> kScenes[] = {
            { "Model scene", SceneType::Model },
            { "Triangles scene", SceneType::Triangles },
        };
        if (ImGui::BeginCombo("Scene", kScenes[(int)currSceneType].name)) {
            for (auto& elm : kScenes) {
                if (ImGui::Selectable(elm.name, currSceneType == elm.value)) {
                    currSceneType = elm.value;
                }
            }
            ImGui::EndCombo();
        }

        auto& currScene = GetCurrentScene();
        if (ImGui::TreeNode("Renderer Info")) {
            ImGui::Text("Canvas size: { %d, %d }", canvasSize.width, canvasSize.height);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Scene Info")) {
            // TODO
            ImGui::TreePop();
        }

        bool readyToRender = currScene.IsReady();
        if (ImGui::Button("Render frame", !readyToRender)) {
            RenderCurrentScene();
            UploadBuffers();
        }
        if (!readyToRender) {
            ImGui::SameLine();
            ImGui::TextUnformatted("The current scene is invalid");
        }
    }

    struct {
        struct {
            RgbaColor clearColor = RgbaColor(255, 255, 255);
            float clearDepth = 0.0f;
        } clear;

        struct {
            ImVec2 ptBegin;
            ImVec2 ptEnd;
            ImVec4 color;
        } line;

        struct ImVec2 {

        } triangle;
    } data_ShowRendererOneOffTools;

    void ShowRendererOneOffTools() {
        auto& dd = data_ShowRendererOneOffTools;

        if (ImGui::CollapsingHeader("Clear")) {
            auto& dc = dd.clear;
            if (ImGui::ColorEdit4("Clear color", &dc.clearColor)) {
                canvas.ClearColor(dc.clearColor);
                UploadBuffers();
            }
            if (ImGui::InputFloat("Clear depth", &dc.clearDepth)) {
                canvas.ClearDepth(dc.clearDepth);
                UploadBuffers();
            }
        }
        if (ImGui::CollapsingHeader("Line")) {
            auto& dc = dd.line;
            ImGui::InputFloat2("Point 1", &dc.ptBegin.x);
            ImGui::InputFloat2("Point 2", &dc.ptEnd.x);
            ImGui::ColorEdit4("Color", &dc.color.x);
            if (ImGui::Button("Draw line")) {
                glm::vec3 vertices[] = {
                    glm::vec3(dc.ptBegin.x, dc.ptBegin.y, 0.0f),
                    glm::vec3(dc.ptEnd.x, dc.ptEnd.y, 0.0f),
                };
                RgbaColor color = Conv::ImVec4_To_RgbaColor(dc.color);
                rasterizer.DrawLine(vertices, color);
                ::UploadTexture(texture, canvas.pixels.data(), canvasSize);
            }
        }
        if (ImGui::CollapsingHeader("Triangle")) {
            auto& dc = dd.triangle;
            // TODO
        }
    }

    void ShowModelEditor() {
        ImGui::ColorEdit4("Clear color", &rd.clearColor);
        ImGui::InputFloat("Clear depth", &rd.clearDepth);

        if (ImGui::Button("Load mesh")) {
            pfd::open_file dialog("Select model file");
            auto result = dialog.result();

            if (!result.empty()) {
                auto& mesh = rd.mesh;
                auto& path = rd.meshFilePath;

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

    void ShowTriangleEditor() {
        ImGui::ColorEdit4("Clear color", &tsd.clearColor);
        ImGui::InputFloat("Clear depth", &tsd.clearDepth);

        if (tsd.currentSelectedVertex != -1) {
            // TODO
        } else {
            ImGui::TextUnformatted("Select an vertex to edit its properties");
        }

        ImGui::Begin("Triangles: scene editor");
        {
            auto dl = ImGui::GetWindowDrawList();

            // Render all current triangles
            for (int i = 0, di = 0; i < tsd.positions.size(); ++i, di += 3) {
                glm::vec3* triangle = &tsd.positions[i];
                RgbaColor* colors = &tsd.colors[di];

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
};

App::App()
    : m{ new Private() } //
{
    constexpr int kWidth = 640;
    constexpr int kHeight = 320;
    m->ResizeCanvas({ kWidth, kHeight });
    m->canvas.ClearColor(RgbaColor(255, 255, 255));
    m->UploadBuffers();
}

App::~App() {
    delete m;
}

void App::Show() {
    ImGui::Begin("Renderer Setup");
    if (ImGui::BeginTabBar("##")) {
        if (ImGui::BeginTabItem("Scene")) {
            if (ImGui::BeginTabBar("##")) {
                if (ImGui::BeginTabItem("Render")) {
                    m->ShowRendererEditor();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Model Scene Setup")) {
                    m->ShowModelEditor();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Triangles Scene Setup")) {
                    m->ShowTriangleEditor();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("One-offs")) {
            m->ShowRendererOneOffTools();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

    ImGui::Begin("Renderer Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImVec2 canvasPos = ImGui::GetCursorPos();
    ImVec2 canvasSize(m->canvasSize.width, m->canvasSize.height);
    ImGui::Image(reinterpret_cast<void*>(m->texture), canvasSize);
    ImRect canvasRect(canvasPos, canvasPos + canvasSize);
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        auto mousePos = ImGui::GetMousePos();
        if (canvasRect.Contains(mousePos)) {
            // TODO zoom
        }
    }
    ImGui::End();

    ImGui::ShowNotifications();
}

void App::RenderFrame() {
    m->UploadBuffers();
}
