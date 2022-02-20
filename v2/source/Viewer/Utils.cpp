#include "Utils.hpp"

#include "Color.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

void ImGui::SetNextWindowSizeRelScreen(float xPercent, float yPercent, ImGuiCond cond) {
    auto vs = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowSize({ vs.x * xPercent, vs.y * yPercent }, cond);
}

void ImGui::SetNextWindowCentered(ImGuiCond cond) {
    auto vs = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos({ vs.x / 2, vs.y / 2 }, cond, { 0.5f, 0.5f });
}

void ImGui::PushDisabled() {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f * ImGui::GetStyle().Alpha);
}

void ImGui::PopDisabled() {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

bool ImGui::Button(const char* label, bool disabled) {
    return Button(label, ImVec2{}, disabled);
}

bool ImGui::Button(const char* label, const ImVec2& sizeArg, bool disabled) {
    if (disabled) PushDisabled();
    bool res = ImGui::Button(label, sizeArg);
    if (disabled) PopDisabled();

    return res;
}

#define EDIT_RGBA_COLOR(EditorFunction, kUsesAlpha)                        \
    float components[4];                                                   \
    components[0] = color->GetNormalizedRed();                             \
    components[1] = color->GetNormalizedGreen();                           \
    components[2] = color->GetNormalizedBlue();                            \
    if constexpr (kUsesAlpha) components[3] = color->GetNormalizedAlpha(); \
    if (EditorFunction(label, components, flags)) {                        \
        color->r = components[0] * 255;                                    \
        color->g = components[1] * 255;                                    \
        color->b = components[2] * 255;                                    \
        if constexpr (kUsesAlpha) color->a = components[3] * 255;          \
        return true;                                                       \
    } else {                                                               \
        return false;                                                      \
    }

bool ImGui::ColorEdit3(const char* label, RgbaColor* color, ImGuiColorEditFlags flags) {
    EDIT_RGBA_COLOR(ColorEdit3, false);
}

bool ImGui::ColorEdit4(const char* label, RgbaColor* color, ImGuiColorEditFlags flags) {
    EDIT_RGBA_COLOR(ColorEdit4, true);
}

bool ImGui::ColorPicker3(const char* label, RgbaColor* color, ImGuiColorEditFlags flags) {
    EDIT_RGBA_COLOR(ColorPicker3, false);
}

bool ImGui::ColorPicker4(const char* label, RgbaColor* color, ImGuiColorEditFlags flags) {
    EDIT_RGBA_COLOR(ColorPicker4, true);
}

#undef EDIT_RGBA_COLOR

struct InputTextCallbackUserData {
    std::string* str;
    ImGuiInputTextCallback chainCallback;
    void* chainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    auto user_data = (InputTextCallbackUserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        auto str = user_data->str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    } else if (user_data->chainCallback) {
        // Forward to user callback, if any
        data->UserData = user_data->chainCallbackUserData;
        return user_data->chainCallback(data);
    }
    return 0;
}

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallbackUserData cbUserData;
    cbUserData.str = str;
    cbUserData.chainCallback = callback;
    cbUserData.chainCallbackUserData = user_data;
    return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cbUserData);
}

bool ImGui::InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallbackUserData cbUserData;
    cbUserData.str = str;
    cbUserData.chainCallback = callback;
    cbUserData.chainCallbackUserData = userData;
    return InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cbUserData);
}

bool ImGui::InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallbackUserData cbUserData;
    cbUserData.str = str;
    cbUserData.chainCallback = callback;
    cbUserData.chainCallbackUserData = userData;
    return InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cbUserData);
}

bool ImGui::Splitter(bool splitVertically, float thickness, float* size1, float* size2, float minSize1, float minSize2, float splitterLongAxisSize) {
    // Adapted from https://github.com/thedmd/imgui-node-editor/blob/master/examples/blueprints-example/blueprints-example.cpp
    // ::Splitter

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (splitVertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(splitVertically ? ImVec2(thickness, splitterLongAxisSize) : ImVec2(splitterLongAxisSize, thickness), 0.0f, 0.0f);

    // Adapted from ImGui::SplitterBehavior, changes:
    // - Simplified unneeded logic (hover_extend and hover_visibility_delay)
    // - Changed clamped delta to clamping result size1 and deriving size2 from size1, allowing automatically adapting to the latest window content region width

    auto itemFlagsBackup = g.CurrentItemFlags;
    g.CurrentItemFlags |= ImGuiItemFlags_NoNav | ImGuiItemFlags_NoNavDefaultFocus;
    bool itemAdd = ItemAdd(bb, id);
    g.CurrentItemFlags = itemFlagsBackup;
    if (!itemAdd) {
        return false;
    }

    bool hovered, held;
    auto bbInteract = bb;
    ButtonBehavior(bbInteract, id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap);
    if (hovered) {
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredRect;
    } // for IsItemHovered(), because bbInteract is larger than bb
    if (g.ActiveId != id) {
        SetItemAllowOverlap();
    }

    if (held || (hovered && g.HoveredIdPreviousFrame == id && g.HoveredIdTimer >= 0.0f)) {
        SetMouseCursor((splitVertically ? ImGuiAxis_X : ImGuiAxis_Y) == ImGuiAxis_Y ? ImGuiMouseCursor_ResizeNS : ImGuiMouseCursor_ResizeEW);
    }

    float contentSize = splitVertically ? window->ContentRegionRect.GetWidth() : window->ContentRegionRect.GetHeight();
    if (held) {
        ImVec2 mouseDelta2D = g.IO.MousePos - g.ActiveIdClickOffset - bbInteract.Min;
        float mouseDelta = ((splitVertically ? ImGuiAxis_X : ImGuiAxis_Y) == ImGuiAxis_Y) ? mouseDelta2D.y : mouseDelta2D.x;

        // Apply resize
        if (mouseDelta != 0.0f) {
            *size1 = ImClamp(*size1 + mouseDelta, minSize1, contentSize - minSize2 - thickness);
            *size2 = contentSize - *size1 - thickness;
            MarkItemEdited(id);
        }
    }

    ImU32 col;
    if (held) {
        col = GetColorU32(ImGuiCol_SeparatorActive);
    } else if (hovered && g.HoveredIdTimer >= 0.0f) {
        col = GetColorU32(ImGuiCol_SeparatorHovered);
    } else {
        col = GetColorU32(ImGuiCol_Separator);
    }
    window->DrawList->AddRectFilled(bb.Min, bb.Max, col, 0.0f);

    return held;
}

RgbaColor Conv::ImU32_To_RgbaColor(ImU32 color) {
    return RgbaColor(static_cast<uint32_t>(color));
}

RgbaColor Conv::ImVec4_To_RgbaColor(ImVec4 color) {
    return RgbaColor(color.x, color.y, color.z, color.w);
}

ImU32 Conv::RgbaColor_To_ImU32(RgbaColor color) {
    ImU32 scalar = 0;
    scalar |= color.r << IM_COL32_R_SHIFT;
    scalar |= color.g << IM_COL32_G_SHIFT;
    scalar |= color.b << IM_COL32_B_SHIFT;
    scalar |= color.a << IM_COL32_A_SHIFT;
    return scalar;
}

ImVec4 Conv::RgbaColor_To_ImVec4(RgbaColor color) {
    return ImVec4{
        color.GetNormalizedRed(),
        color.GetNormalizedGreen(),
        color.GetNormalizedBlue(),
        color.GetNormalizedAlpha(),
    };
}

ImU32 Conv::Components_To_ImU32(int r, int g, int b, int a) {
    return RgbaColor_To_ImU32(RgbaColor(r, g, b, a));
}

ImVec4 Conv::Components_To_ImVec4(int r, int g, int b, int a) {
    return RgbaColor_To_ImVec4(RgbaColor(r, g, b, a));
}
