#pragma once

namespace grit {

/// \ingroup grit-audio-stereo
template<typename Float>
struct StereoFrame
{
    using SampleType = Float;

    friend constexpr auto operator+(StereoFrame lhs, Float rhs) -> StereoFrame
    {
        return {
            lhs.left + rhs,
            lhs.right + rhs,
        };
    }

    friend constexpr auto operator-(StereoFrame lhs, Float rhs) -> StereoFrame
    {
        return {
            lhs.left - rhs,
            lhs.right - rhs,
        };
    }

    friend constexpr auto operator*(StereoFrame lhs, Float rhs) -> StereoFrame
    {
        return {
            lhs.left * rhs,
            lhs.right * rhs,
        };
    }

    friend constexpr auto operator/(StereoFrame lhs, Float rhs) -> StereoFrame
    {
        return {
            lhs.left / rhs,
            lhs.right / rhs,
        };
    }

    friend constexpr auto operator+(StereoFrame lhs, StereoFrame rhs) -> StereoFrame
    {
        return {
            lhs.left + rhs.left,
            lhs.right + rhs.right,
        };
    }

    friend constexpr auto operator-(StereoFrame lhs, StereoFrame rhs) -> StereoFrame
    {
        return {
            lhs.left - rhs.left,
            lhs.right - rhs.right,
        };
    }

    friend constexpr auto operator*(StereoFrame lhs, StereoFrame rhs) -> StereoFrame
    {
        return {
            lhs.left * rhs.left,
            lhs.right * rhs.right,
        };
    }

    friend constexpr auto operator/(StereoFrame lhs, StereoFrame rhs) -> StereoFrame
    {
        return {
            lhs.left / rhs.left,
            lhs.right / rhs.right,
        };
    }

    friend constexpr auto operator+=(StereoFrame& lhs, StereoFrame rhs) -> StereoFrame&
    {
        lhs = lhs + rhs;
        return lhs;
    }

    friend constexpr auto operator-=(StereoFrame& lhs, StereoFrame rhs) -> StereoFrame&
    {
        lhs = lhs - rhs;
        return lhs;
    }

    friend constexpr auto operator*=(StereoFrame& lhs, StereoFrame rhs) -> StereoFrame&
    {
        lhs = lhs * rhs;
        return lhs;
    }

    friend constexpr auto operator/=(StereoFrame& lhs, StereoFrame rhs) -> StereoFrame&
    {
        lhs = lhs / rhs;
        return lhs;
    }

    friend constexpr auto operator+=(StereoFrame& lhs, Float rhs) -> StereoFrame&
    {
        lhs = lhs + rhs;
        return lhs;
    }

    friend constexpr auto operator-=(StereoFrame& lhs, Float rhs) -> StereoFrame&
    {
        lhs = lhs - rhs;
        return lhs;
    }

    friend constexpr auto operator*=(StereoFrame& lhs, Float rhs) -> StereoFrame&
    {
        lhs = lhs * rhs;
        return lhs;
    }

    friend constexpr auto operator/=(StereoFrame& lhs, Float rhs) -> StereoFrame&
    {
        lhs = lhs / rhs;
        return lhs;
    }

    Float left{};
    Float right{};
};

}  // namespace grit
