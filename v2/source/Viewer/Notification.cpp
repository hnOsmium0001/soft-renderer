// Adapted from https://github.com/patrickcjk/imgui-notify
#include "Notification.hpp"

#include "Macros.hpp"

// #include <IconsFontAwesome.h>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <utility>
#include <vector>

ImGuiToast::ImGuiToast(ImGuiToastType type, int dismissTime) {
    IM_ASSERT(type < ImGuiToastType_COUNT);

    mType = type;
    mDismissTime = dismissTime;

    using namespace std::chrono;
    auto timeStamp = system_clock::now().time_since_epoch();
    mCreationTime = duration_cast<milliseconds>(timeStamp).count();

    memset(mTitle, 0, sizeof(mTitle));
    memset(mContent, 0, sizeof(mContent));
}

ImGuiToast::ImGuiToast(ImGuiToastType type, const char* format, ...)
    : ImGuiToast(type) {
    if (format) {
        va_list args;
        va_start(args, format);
        SetContent(format, args);
        va_end(args);
    }
}

ImGuiToast::ImGuiToast(ImGuiToastType type, int dismissTime, const char* format, ...)
    : ImGuiToast(type, dismissTime) {
    if (format) {
        va_list args;
        va_start(args, format);
        SetContent(format, args);
        va_end(args);
    }
}

void ImGuiToast::SetTitle(const char* format, ...) {
    if (format) {
        va_list args;
        va_start(args, format);
        SetTitle(format, args);
        va_end(args);
    }
}

void ImGuiToast::SetContent(const char* format, ...) {
    if (format) {
        va_list args;
        va_start(args, format);
        SetContent(format, args);
        va_end(args);
    }
}

void ImGuiToast::SetType(const ImGuiToastType& type) {
    IM_ASSERT(type < ImGuiToastType_COUNT);
    mType = type;
}

const char* ImGuiToast::GetTitle() {
    return mTitle;
}

const char* ImGuiToast::GetDefaultTitle() {
    if (!strlen(mTitle)) {
        switch (mType) {
            case ImGuiToastType_None: return nullptr;
            case ImGuiToastType_Success: return "Success";
            case ImGuiToastType_Warning: return "Warning";
            case ImGuiToastType_Error: return "Error";
            case ImGuiToastType_Info: return "Info";
            case ImGuiToastType_COUNT: UNREACHABLE;
        }
    }

    return mTitle;
}

ImGuiToastType ImGuiToast::GetType() {
    return mType;
}

ImVec4 ImGuiToast::GetColor() {
    switch (mType) {
        case ImGuiToastType_None: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
        case ImGuiToastType_Success: return ImVec4(0, 1.0f, 0, 1.0f); // Green
        case ImGuiToastType_Warning: return ImVec4(1.0f, 1.0f, 0, 1.0f); // Yellow
        case ImGuiToastType_Error: return ImVec4(1.0f, 0, 0, 1.0f); // Red
        case ImGuiToastType_Info: return ImVec4(0, 0.616, 1.0f, 1.0f); // Blue
        case ImGuiToastType_COUNT: UNREACHABLE;
    }
    return ImVec4();
}

const char* ImGuiToast::GetIcon() {
    switch (mType) {
        case ImGuiToastType_None: return nullptr;
#if 1
        // TODO add IconFontHeaders and replace with proper icons
        case ImGuiToastType_Success: return nullptr;
        case ImGuiToastType_Warning: return nullptr;
        case ImGuiToastType_Error: return nullptr;
        case ImGuiToastType_Info: return nullptr;
#else
        case ImGuiToastType_Success: return ICON_FA_CHECK_CIRCLE;
        case ImGuiToastType_Warning: return ICON_FA_EXCLAMATION_TRIANGLE;
        case ImGuiToastType_Error: return ICON_FA_TIMES_CIRCLE;
        case ImGuiToastType_Info: return ICON_FA_INFO_CIRCLE;
#endif
        case ImGuiToastType_COUNT: UNREACHABLE;
    }
    return nullptr;
}

const char* ImGuiToast::GetContent() {
    return this->mContent;
}

uint64_t ImGuiToast::GetElapsedTime() {
    using namespace std::chrono;
    auto timeStamp = system_clock::now().time_since_epoch();
    auto timeStampI = duration_cast<milliseconds>(timeStamp).count();
    return timeStampI - mCreationTime;
}

ImGuiToastPhase ImGuiToast::GetPhase() {
    const auto elapsed = GetElapsedTime();

    if (elapsed > kNotifyFadeInOutTime + mDismissTime + kNotifyFadeInOutTime) {
        return ImGuiToastPhase_Expired;
    } else if (elapsed > kNotifyFadeInOutTime + mDismissTime) {
        return ImGuiToastPhase_FadeOut;
    } else if (elapsed > kNotifyFadeInOutTime) {
        return ImGuiToastPhase_Wait;
    } else {
        return ImGuiToastPhase_FadeIn;
    }
}

float ImGuiToast::GetFadePercent() {
    const auto phase = GetPhase();
    const auto elapsed = GetElapsedTime();

    if (phase == ImGuiToastPhase_FadeIn)
    {
        return ((float)elapsed / (float)kNotifyFadeInOutTime) * kNotifyOpacity;
    } else if (phase == ImGuiToastPhase_FadeOut)
    {
        return (1.0f - (((float)elapsed - (float)kNotifyFadeInOutTime - (float)mDismissTime) / (float)kNotifyFadeInOutTime)) * kNotifyOpacity;
    }

    return 1.0f * kNotifyOpacity;
}

void ImGuiToast::SetTitle(const char* format, va_list args) {
    vsnprintf(mTitle, sizeof(mTitle), format, args);
}

void ImGuiToast::SetContent(const char* format, va_list args) {
    vsnprintf(mContent, sizeof(mContent), format, args);
}

namespace ImGui {
static std::vector<ImGuiToast> notifications;
}

static bool IsNullOrEmpty(const char* str) {
    return !str || !strlen(str);
}

void ImGui::AddNotification(ImGuiToast toast) {
    notifications.push_back(std::move(toast));
}

void ImGui::RemoveNotification(int index) {
    notifications.erase(notifications.begin() + index);
}

void ImGui::ShowNotifications() {
    auto vpSize = GetMainViewport()->Size;

    float height = 0.0f;
    for (auto i = 0; i < notifications.size(); i++) {
        auto* currentToast = &notifications[i];

        // Remove toast if expired
        if (currentToast->GetPhase() == ImGuiToastPhase_Expired) {
            RemoveNotification(i);
            continue;
        }

        // Get icon, title and other data
        const auto icon = currentToast->GetIcon();
        const auto title = currentToast->GetTitle();
        const auto content = currentToast->GetContent();
        const auto defaultTitle = currentToast->GetDefaultTitle();
        const auto opacity = currentToast->GetFadePercent(); // Get opacity based of the current phase

        // Window rendering
        auto textColor = currentToast->GetColor();
        textColor.w = opacity;

        // Generate new unique name for this toast
        char windowName[50];
        snprintf(windowName, std::size(windowName), "##TOAST%d", i);

        SetNextWindowBgAlpha(opacity);
        SetNextWindowPos(ImVec2(vpSize.x - kNotifyPaddingX, vpSize.y - kNotifyPaddingY - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        Begin(windowName, nullptr, kNotifyToastFlags);

        // Here we render the toast content
        {
            PushTextWrapPos(vpSize.x / 3.0f); // We want to support multi-line text, this will wrap the text after 1/3 of the screen width

            bool wasTitleRendered = false;

            // If an icon is set
            if (!::IsNullOrEmpty(icon)) {
                // Render icon text
                PushStyleColor(ImGuiCol_Text, textColor);
                TextUnformatted(icon);
                PopStyleColor();
                wasTitleRendered = true;
            }

            // If a title is set
            if (!::IsNullOrEmpty(title)) {
                // If a title and an icon is set, we want to render on same line
                if (!::IsNullOrEmpty(icon))
                    SameLine();

                TextUnformatted(title); // Render title text
                wasTitleRendered = true;
            } else if (!::IsNullOrEmpty(defaultTitle)) {
                if (!::IsNullOrEmpty(icon))
                    SameLine();

                TextUnformatted(defaultTitle); // Render default title text (ImGuiToastType_Success -> "Success", etc...)
                wasTitleRendered = true;
            }

            // In case ANYTHING was rendered in the top, we want to add a small padding so the text (or icon) looks centered vertically
            if (wasTitleRendered && !::IsNullOrEmpty(content)) {
                SetCursorPosY(GetCursorPosY() + 5.0f); // Must be a better way to do this!!!!
            }

            // If a content is set
            if (!::IsNullOrEmpty(content)) {
                if (wasTitleRendered) {
                    Separator();
                }

                TextUnformatted(content); // Render content text
            }

            PopTextWrapPos();
        }

        // Save height for next toasts
        height += GetWindowHeight() + kNotifyPaddingMessageY;

        End();
    }
}
