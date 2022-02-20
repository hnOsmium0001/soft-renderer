// Adapted from https://github.com/patrickcjk/imgui-notify
#pragma once

#include <imgui.h>
#include <cstdint>

enum ImGuiToastType {
    ImGuiToastType_None,
    ImGuiToastType_Success,
    ImGuiToastType_Warning,
    ImGuiToastType_Error,
    ImGuiToastType_Info,
    ImGuiToastType_COUNT
};

enum ImGuiToastPhase {
    ImGuiToastPhase_FadeIn,
    ImGuiToastPhase_Wait,
    ImGuiToastPhase_FadeOut,
    ImGuiToastPhase_Expired,
    ImGuiToastPhase_COUNT
};

enum ImGuiToastPos {
    ImGuiToastPos_TopLeft,
    ImGuiToastPos_TopCenter,
    ImGuiToastPos_TopRight,
    ImGuiToastPos_BottomLeft,
    ImGuiToastPos_BottomCenter,
    ImGuiToastPos_BottomRight,
    ImGuiToastPos_Center,
    ImGuiToastPos_COUNT
};

constexpr int kNotifyMaxMsgLength = 4096; // Max message content length
constexpr float kNotifyPaddingX = 20.0f; // Bottom-left X padding
constexpr float kNotifyPaddingY = 20.0f; // Bottom-left Y padding
constexpr float kNotifyPaddingMessageY = 10.0f; // Padding Y between each message
constexpr uint64_t kNotifyFadeInOutTime = 150; // Fade in and out duration
constexpr uint64_t kNotifyDefaultDismiss = 3000; // Auto dismiss after X ms (default, applied only of no data provided in constructors)
constexpr float kNotifyOpacity = 1.0f; // 0-1 Toast opacity
constexpr ImGuiWindowFlags kNotifyToastFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;

class ImGuiToast {
private:
    ImGuiToastType mType = ImGuiToastType_None;
    char mTitle[kNotifyMaxMsgLength] = {};
    char mContent[kNotifyMaxMsgLength] = {};
    int mDismissTime = kNotifyDefaultDismiss;
    uint64_t mCreationTime = 0;

public:
    ImGuiToast(ImGuiToastType type, int dismissTime = kNotifyDefaultDismiss);
    ImGuiToast(ImGuiToastType type, const char* format, ...);
    ImGuiToast(ImGuiToastType type, int dismissTime, const char* format, ...);

    void SetTitle(const char* format, ...);
    void SetContent(const char* format, ...);
    void SetType(const ImGuiToastType& type);

    const char* GetTitle();
    const char* GetDefaultTitle();
    ImGuiToastType GetType();
    ImVec4 GetColor();
    const char* GetIcon();
    const char* GetContent();
    ;
    uint64_t GetElapsedTime();
    ImGuiToastPhase GetPhase();
    float GetFadePercent();

private:
    void SetTitle(const char* format, va_list args);
    void SetContent(const char* format, va_list args);
};

namespace ImGui {
void AddNotification(ImGuiToast toast);
void RemoveNotification(int index);
void ShowNotifications();
} // namespace ImGui
