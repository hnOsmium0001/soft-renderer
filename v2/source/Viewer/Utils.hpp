#pragma once

#include "Size.hpp"
#include "all_fwd.hpp"

#include <imgui.h>
#include <glm/glm.hpp>
#include <string>

template <class TEnum>
struct EnumElement {
    const char* name;
    TEnum value;
};

namespace ImGui {
void SetNextWindowSizeRelScreen(float xPercent, float yPercent, ImGuiCond cond = ImGuiCond_None);
void SetNextWindowCentered(ImGuiCond cond = ImGuiCond_None);

void PushDisabled();
void PopDisabled();

bool Button(const char* label, bool disabled);
bool Button(const char* label, const ImVec2& sizeArg, bool disabled);

bool ColorEdit3(const char* label, RgbaColor* color, ImGuiColorEditFlags flags = 0);
bool ColorEdit4(const char* label, RgbaColor* color, ImGuiColorEditFlags flags = 0);
bool ColorPicker3(const char* label, RgbaColor* color, ImGuiColorEditFlags flags = 0);
bool ColorPicker4(const char* label, RgbaColor* color, ImGuiColorEditFlags flags = 0);

bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* userData = nullptr);
bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* userData = nullptr);

bool Splitter(bool splitVertically, float thickness, float* size1, float* size2, float minSize1, float minSize2, float splitterLongAxisSize = -1.0f);
} // namespace ImGui

namespace Conv {
RgbaColor ImU32_To_RgbaColor(ImU32 color);
RgbaColor ImVec4_To_RgbaColor(ImVec4 color);
ImU32 RgbaColor_To_ImU32(RgbaColor color);
ImVec4 RgbaColor_To_ImVec4(RgbaColor color);

ImU32 Components_To_ImU32(int r, int g, int b, int a);
ImVec4 Components_To_ImVec4(int r, int g, int b, int a);

template <class T>
glm::vec<2, T> ImVec2_To_Glm(ImVec2 vec) {
    return { (T)vec.x, (T)vec.y };
}

template <class T>
ImVec2 Glm_To_ImVec2(glm::vec<2, T> vec) {
    return ImVec2(vec.x, vec.y);
}

template <class T>
Size2<T> ImVec2_To_Size2(ImVec2 vec) {
    return { (T)vec.x, (T)vec.y };
}

template <class T>
ImVec2 Size2_To_ImVec2(Size2<T> size) {
    return ImVec2(size.width, size.height);
}
} // namespace Conv
