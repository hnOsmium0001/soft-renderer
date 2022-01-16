#pragma once

#include "Margin.hpp"
#include "Size.hpp"

#include <glm/glm.hpp>

/// Rect is a rectangle representation based on a point and a dimensions, in television coordinate space
/// (x increases from left to right, y increases from top to bottom).
template <class T>
class Rect {
public:
    using ScalarType = T;
    using VectorType = glm::vec<2, T>;

public:
    T x;
    T y;
    T width;
    T height;

public:
    Rect()
        : x{ 0 }, y{ 0 }, width{ 0 }, height{ 0 } {
    }

    Rect(T x, T y, T width, T height)
        : x{ x }, y{ y }, width{ width }, height{ height } {
    }

    Rect(VectorType pos, Size2<T> size)
        : x{ pos.x }
        , y{ pos.y }
        , width{ size.width }
        , height{ size.height } {
    }

    Rect(VectorType topLeft, VectorType bottomRight)
        : x{ topLeft.x }
        , y{ topLeft.y }
        , width{ bottomRight.x - topLeft.x }
        , height{ bottomRight.y - topLeft.y } {
    }

    T x0() const { return x; }
    T y0() const { return y; }
    T x1() const { return x + width; }
    T y1() const { return y + height; }

    VectorType TopLeft() const {
        return VectorType{ x, y };
    }

    VectorType TopRight() const {
        return VectorType{ x + width, y };
    }

    VectorType BottomLeft() const {
        return VectorType{ x, y + height };
    }

    VectorType BottomRight() const {
        return VectorType{ x + width, y + height };
    }

    VectorType Center() const {
        return TopLeft() + VectorType{ width / 2, height / 2 };
    }

    Size2<T> Dimensions() const {
        return Size2<T>{ width, height };
    }

    Size2<T> Extents() const {
        return Size2<T>{ width / 2, height / 2 };
    }

    /// Assumes `bySize * 2` is smaller than both `width` and `height` (does not produce a negative-dimension rectangle).
    Rect Shrink(T bySize) const {
        T two = bySize * 2;
        return Rect{ x + bySize, y + bySize, width - two, height - two };
    }

    Rect Shrink(T left, T top, T right, T bottom) const {
        return Rect{
            x + left,
            y + top,
            width - left - right,
            height - top - bottom,
        };
    }

    Rect Shrink(const Margin<T>& m) const {
        return Shrink(m.left, m.top, m.right, m.bottom);
    }

    Rect Expand(T bySize) const {
        T two = bySize * 2;
        return Rect{ x - bySize, y - bySize, width + two, height + two };
    }

    Rect Expand(T left, T top, T right, T bottom) const {
        return Rect{
            x - left,
            y - top,
            width + left + right,
            height + top + bottom,
        };
    }

    Rect Expand(const Margin<T>& m) const {
        return Expand(m.left, m.top, m.right, m.bottom);
    }

    bool Contains(VectorType point) const {
        return point.x >= x &&
            point.y >= y &&
            point.x < x + width &&
            point.y < y + height;
    }

    bool Intersects(const Rect& that) const {
        bool xBetween = x > that.x0() && x < that.x1();
        bool yBetween = y > that.y0() && y < that.y1();
        return xBetween && yBetween;
    }

    // Write min()/max() tenary by hand so that we don't have to include <algorithm>
    // This file is practically going to be included in every file in this project

    static Rect Intersection(const Rect& a, const Rect& b) {
        auto x0 = a.x0() > b.x0() ? a.x0() : b.x0(); // Max
        auto y0 = a.y0() > b.y0() ? a.y0() : b.y0(); // Max
        auto x1 = a.x1() < b.x1() ? a.x1() : b.x1(); // Min
        auto y1 = a.y1() < b.y1() ? a.y1() : b.y1(); // Min
        auto width = x1 - x0;
        auto height = y1 - x0;
        return Rect{ x0, y0, width, height };
    }

    static Rect Union(const Rect& a, const Rect& b) {
        auto x0 = a.x0() < b.x0() ? a.x0() : b.x0(); // Min
        auto y0 = a.y0() < b.y0() ? a.y0() : b.y0(); // Min
        auto x1 = a.x1() > b.x1() ? a.x1() : b.x1(); // Max
        auto y1 = a.y1() > b.y1() ? a.y1() : b.y1(); // Max
        auto width = x1 - x0;
        auto height = y1 - x0;
        return Rect{ x0, y0, width, height };
    }

    friend bool operator==(const Rect<T>&, const Rect<T>&) = default;

    Rect operator+(glm::vec<2, T> offset) const {
        return { x + offset.x, y + offset.y, width, height };
    }

    Rect operator-(glm::vec<2, T> offset) const {
        return { x - offset.x, y - offset.y, width, height };
    }

    Rect& operator+=(glm::vec<2, T> offset) {
        x += offset.x;
        y += offset.y;
        return *this;
    }

    Rect& operator-=(glm::vec<2, T> offset) {
        x -= offset.x;
        y -= offset.y;
        return *this;
    }

    template <class TTarget>
    Rect<TTarget> Cast() const {
        return {
            static_cast<TTarget>(x),
            static_cast<TTarget>(y),
            static_cast<TTarget>(width),
            static_cast<TTarget>(height),
        };
    }
};
