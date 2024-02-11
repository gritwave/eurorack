#pragma once

#include <grit/audio/stereo/stereo_frame.hpp>

namespace grit {

/// \ingroup grit-audio-stereo
template<typename T>
struct StereoWidth
{
    using value_type = T;

    constexpr StereoWidth() = default;

    explicit constexpr StereoWidth(T width) : _width{width}, _coeff{width * T(0.5)} {}

    constexpr auto setWidth(T width) -> void
    {
        _width = width;
        _coeff = width * T(0.5);
    }

    [[nodiscard]] constexpr auto getWidth() const -> T { return _width; }

    [[nodiscard]] constexpr auto operator()(StereoFrame<T> in) -> StereoFrame<T>
    {
        auto const mid  = (in.left + in.right) * 0.5F;
        auto const side = (in.right - in.left) * _coeff;
        return {
            mid - side,
            mid + side,
        };
    }

private:
    T _width{1.0};
    T _coeff{_width * T(0.5)};
};

}  // namespace grit
