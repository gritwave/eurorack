#pragma once

namespace grit {

template<typename SampleType>
struct StereoFrame
{
    using value_type = SampleType;

    SampleType left{};
    SampleType right{};
};

template<typename T>
[[nodiscard]] constexpr auto operator+(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left + rhs.left,
        lhs.right + rhs.right,
    };
}

template<typename T>
[[nodiscard]] constexpr auto operator-(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left - rhs.left,
        lhs.right - rhs.right,
    };
}

template<typename T>
[[nodiscard]] constexpr auto operator*(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left * rhs.left,
        lhs.right * rhs.right,
    };
}

template<typename T>
[[nodiscard]] constexpr auto operator/(StereoFrame<T> lhs, StereoFrame<T> rhs) -> StereoFrame<T>
{
    return {
        lhs.left / rhs.left,
        lhs.right / rhs.right,
    };
}

template<typename T>
[[nodiscard]] constexpr auto operator+(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left + rhs,
        lhs.right + rhs,
    };
}

template<typename T>
[[nodiscard]] constexpr auto operator-(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left - rhs,
        lhs.right - rhs,
    };
}

template<typename T>
[[nodiscard]] constexpr auto operator*(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left * rhs,
        lhs.right * rhs,
    };
}

template<typename T>
[[nodiscard]] constexpr auto operator/(StereoFrame<T> lhs, T rhs) -> StereoFrame<T>
{
    return {
        lhs.left / rhs,
        lhs.right / rhs,
    };
}

template<typename T>
constexpr auto operator+=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs + rhs;
    return lhs;
}

template<typename T>
constexpr auto operator-=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs - rhs;
    return lhs;
}

template<typename T>
constexpr auto operator*=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs * rhs;
    return lhs;
}

template<typename T>
constexpr auto operator/=(StereoFrame<T>& lhs, StereoFrame<T> rhs) -> StereoFrame<T>&
{
    lhs = lhs / rhs;
    return lhs;
}

template<typename T>
constexpr auto operator+=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs + rhs;
    return lhs;
}

template<typename T>
constexpr auto operator-=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs - rhs;
    return lhs;
}

template<typename T>
constexpr auto operator*=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs * rhs;
    return lhs;
}

template<typename T>
constexpr auto operator/=(StereoFrame<T>& lhs, T rhs) -> StereoFrame<T>&
{
    lhs = lhs / rhs;
    return lhs;
}

}  // namespace grit
