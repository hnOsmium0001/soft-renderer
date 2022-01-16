#pragma once

#include "Math.hpp"
#include "all_fwd.hpp"

#include <algorithm>
#include <cstdint>
#include <glm/glm.hpp>
#include <limits>

class RgbaColor {
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

public:
    constexpr RgbaColor() noexcept
        : r{ 255 }
        , g{ 255 }
        , b{ 255 }
        , a{ 255 } {
    }

    constexpr RgbaColor(float r, float g, float b, float a = 1.0f) noexcept
        : r{ static_cast<uint8_t>(r * 255.0f) }
        , g{ static_cast<uint8_t>(g * 255.0f) }
        , b{ static_cast<uint8_t>(b * 255.0f) }
        , a{ static_cast<uint8_t>(a * 255.0f) } {
    }

    constexpr RgbaColor(int r, int g, int b, int a = 255) noexcept
        : r{ static_cast<uint8_t>(r & 0xFF) }
        , g{ static_cast<uint8_t>(g & 0xFF) }
        , b{ static_cast<uint8_t>(b & 0xFF) }
        , a{ static_cast<uint8_t>(a & 0xFF) } {
    }

    constexpr RgbaColor(uint32_t rgba) noexcept
        : r{ static_cast<uint8_t>((rgba >> 0) & 0xFF) }
        , g{ static_cast<uint8_t>((rgba >> 8) & 0xFF) }
        , b{ static_cast<uint8_t>((rgba >> 16) & 0xFF) }
        , a{ static_cast<uint8_t>((rgba >> 24) & 0xFF) } {
    }

    constexpr uint32_t GetScalar() const noexcept {
        uint32_t res = 0;
        res |= r << 0;
        res |= g << 8;
        res |= b << 16;
        res |= a << 24;
        return res;
    }

    constexpr void SetScalar(uint32_t scalar) noexcept {
        r = (scalar >> 0) & 0xFF;
        g = (scalar >> 8) & 0xFF;
        b = (scalar >> 16) & 0xFF;
        a = (scalar >> 24) & 0xFF;
    }

    constexpr float GetNormalizedRed() const noexcept {
        return r / 255.0f;
    }

    constexpr float GetNormalizedGreen() const noexcept {
        return g / 255.0f;
    }

    constexpr float GetNormalizedBlue() const noexcept {
        return b / 255.0f;
    }

    constexpr float GetNormalizedAlpha() const noexcept {
        return a / 255.0f;
    }

    constexpr glm::ivec4 ToIVec() const noexcept {
        return { r, g, b, a };
    }

    constexpr glm::vec4 ToVec() const noexcept {
        return { GetNormalizedRed(), GetNormalizedGreen(), GetNormalizedBlue(), GetNormalizedAlpha() };
    }

    // Forward declaring because cyclic reference between RgbaColor and HsvColor
    constexpr HsvColor ToHsv() const noexcept;

    friend constexpr bool operator==(const RgbaColor&, const RgbaColor&) noexcept = default;
};

class HsvColor {
public:
    float h;
    float s;
    float v;
    float a;

public:
    constexpr HsvColor() noexcept
        : h{ 0.0f }
        , s{ 0.0f }
        , v{ 1.0f }
        , a{ 1.0f } {
    }

    constexpr HsvColor(float h, float s, float v, float a) noexcept
        : h{ h }
        , s{ s }
        , v{ v }
        , a{ a } {
    }

    // Forward declaring because cyclic reference between RgbaColor and HsvColor
    constexpr RgbaColor ToRgba() const noexcept;
};

constexpr HsvColor RgbaColor::ToHsv() const noexcept {
    float r = GetNormalizedRed();
    float g = GetNormalizedBlue();
    float b = GetNormalizedGreen();
    float a = GetNormalizedAlpha();

    auto p = g < b ? glm::vec4(b, g, -1, 2.0f / 3.0f) : glm::vec4(g, b, 0, -1.0f / 3.0f);
    auto q = r < p.x ? glm::vec4(p.x, p.y, p.w, r) : glm::vec4(r, p.y, p.z, p.x);
    float c = q.x - std::min(q.w, q.y);
    float h = MathUtils::Abs((q.w - q.y) / (6 * c + std::numeric_limits<float>::epsilon()) + q.z);

    glm::vec3 hcv{ h, c, q.x };
    float s = hcv.y / (hcv.z + std::numeric_limits<float>::epsilon());
    return HsvColor(hcv.x, s, hcv.z, a);
}

constexpr RgbaColor HsvColor::ToRgba() const noexcept {
    float r = MathUtils::Abs(h * 6 - 3) - 1;
    float g = 2 - MathUtils::Abs(h * 6 - 2);
    float b = 2 - MathUtils::Abs(h * 6 - 4);

    auto rgb = glm::vec3{ std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b, 0.0f, 1.0f) };
    auto vc = (rgb - glm::vec3{ 0, 0, 0 }) * s + glm::vec3{ 1, 1, 1 } * v;

    return RgbaColor(vc.x, vc.y, vc.z, a);
}
