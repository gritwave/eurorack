#pragma once

namespace grit {

/// \ingroup grit-audio-stereo
template<typename SampleType>
struct StereoFrame
{
    using value_type = SampleType;

    SampleType left{};
    SampleType right{};
};

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator+(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left + rhs.left,
        lhs.right + rhs.right,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator-(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left - rhs.left,
        lhs.right - rhs.right,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator*(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left * rhs.left,
        lhs.right * rhs.right,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator/(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left / rhs.left,
        lhs.right / rhs.right,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator+(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left + rhs,
        lhs.right + rhs,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator-(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left - rhs,
        lhs.right - rhs,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator*(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left * rhs,
        lhs.right * rhs,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
[[nodiscard]] constexpr auto operator/(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left / rhs,
        lhs.right / rhs,
    };
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator+=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs + rhs;
    return lhs;
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator-=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs - rhs;
    return lhs;
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator*=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs * rhs;
    return lhs;
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator/=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs / rhs;
    return lhs;
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator+=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs + rhs;
    return lhs;
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator-=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs - rhs;
    return lhs;
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator*=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs * rhs;
    return lhs;
}

/// \ingroup grit-audio-stereo
template<typename T>
constexpr auto operator/=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs / rhs;
    return lhs;
}

}  // namespace grit
