#pragma once

namespace grit {

/// \ingroup grit-audio-stereo
template<typename SampleType>
struct StereoFrame
{
    using value_type = SampleType;

    friend constexpr auto operator+(StereoFrame lhs, SampleType rhs) -> StereoFrame
    {
        return {
            lhs.left + rhs,
            lhs.right + rhs,
        };
    }

    friend constexpr auto operator-(StereoFrame lhs, SampleType rhs) -> StereoFrame
    {
        return {
            lhs.left - rhs,
            lhs.right - rhs,
        };
    }

    friend constexpr auto operator*(StereoFrame lhs, SampleType rhs) -> StereoFrame
    {
        return {
            lhs.left * rhs,
            lhs.right * rhs,
        };
    }

    friend constexpr auto operator/(StereoFrame lhs, SampleType rhs) -> StereoFrame
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

    friend constexpr auto operator+=(StereoFrame& lhs, SampleType rhs) -> StereoFrame&
    {
        lhs = lhs + rhs;
        return lhs;
    }

    friend constexpr auto operator-=(StereoFrame& lhs, SampleType rhs) -> StereoFrame&
    {
        lhs = lhs - rhs;
        return lhs;
    }

    friend constexpr auto operator*=(StereoFrame& lhs, SampleType rhs) -> StereoFrame&
    {
        lhs = lhs * rhs;
        return lhs;
    }

    friend constexpr auto operator/=(StereoFrame& lhs, SampleType rhs) -> StereoFrame&
    {
        lhs = lhs / rhs;
        return lhs;
    }

    SampleType left{};
    SampleType right{};
};

}  // namespace grit
