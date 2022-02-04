#pragma once

namespace MathUtils {

// `std::abs` is currently not constexpr (though there is no reason that it can't)
// so we use a custom function that is basically what every implementation uses for std::abs, except marked constexpr
// See https://stackoverflow.com/questions/27708629/why-isnt-abs-constexpr
template <class T>
constexpr T Abs(T v) noexcept {
    return v < 0.0f ? -v : v;
}

} // namespace MathUtils
