#pragma once

#include <glm/glm.hpp>

template <class T>
class Size2 {
public:
    T width;
    T height;

public:
    Size2()
        : width{ 0 }, height{ 0 } {
    }

    Size2(T width, T height)
        : width{ width }, height{ height } {
    }

    Size2(glm::vec<2, T> vec)
        : width{ vec.x }, height{ vec.y } {
    }

    operator glm::vec<2, T>() const {
        return { width, height };
    }

    glm::vec<2, T> AsVec() const {
        return { width, height };
    }

    T Area() const {
        return width * height;
    }

    T Perimeter() const {
        return 2 * width + 2 * height;
    }

    friend bool operator==(const Size2<T>&, const Size2<T>&) = default;

    template <class TTarget>
    Size2<TTarget> Cast() const {
        return {
            static_cast<TTarget>(width),
            static_cast<TTarget>(height),
        };
    }
};

template <class T>
Size2<T> operator+(Size2<T> a, Size2<T> b) {
    return { a.width + b.width, a.height + b.height };
}

template <class T>
Size2<T> operator-(Size2<T> a, Size2<T> b) {
    return { a.width - b.width, a.height - b.height };
}

template <class T, class N>
auto operator*(Size2<T> a, N mult) -> Size2<decltype(a.width * mult)> {
    return { a.width * mult, a.height * mult };
}

template <class T, class N>
auto operator/(Size2<T> a, N mult) -> Size2<decltype(a.width / mult)> {
    return { a.width / mult, a.height / mult };
}
