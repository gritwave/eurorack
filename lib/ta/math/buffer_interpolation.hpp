#pragma once

#include <ta/math/fast_lerp.hpp>
#include <ta/math/hermite_interpolation.hpp>

#include <etl/span.hpp>

namespace ta
{

struct BufferInterpolation
{
    struct None
    {
        template<typename SampleType, etl::size_t Extent>
        [[nodiscard]] constexpr auto operator()(etl::span<SampleType const, Extent> buffer, etl::size_t readPos,
                                                SampleType fracPos) -> SampleType
        {
            etl::ignore_unused(fracPos);
            return buffer[readPos % buffer.size()];
        }
    };

    struct Linear
    {
        template<typename SampleType, etl::size_t Extent>
        [[nodiscard]] constexpr auto operator()(etl::span<SampleType const, Extent> buffer, etl::size_t readPos,
                                                SampleType fracPos) -> SampleType
        {
            auto const x0 = buffer[readPos % buffer.size()];
            auto const x1 = buffer[(readPos + 1) % buffer.size()];
            return fast_lerp(x0, x1, fracPos);
        }
    };

    struct Hermite
    {
        template<typename SampleType, etl::size_t Extent>
        [[nodiscard]] constexpr auto operator()(etl::span<SampleType const, Extent> buffer, etl::size_t readPos,
                                                SampleType fracPos) -> SampleType
        {
            auto const pos = readPos + buffer.size();
            auto const xm1 = buffer[(pos - 1) % buffer.size()];
            auto const x0  = buffer[pos % buffer.size()];
            auto const x1  = buffer[(pos + 1) % buffer.size()];
            auto const x2  = buffer[(pos + 2) % buffer.size()];
            return hermite_interpolation(xm1, x0, x1, x2, fracPos);
        }
    };
};

}  // namespace ta
