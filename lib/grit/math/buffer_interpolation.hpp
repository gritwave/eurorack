#pragma once

#include <grit/core/mdspan.hpp>
#include <grit/math/fast_lerp.hpp>
#include <grit/math/hermite_interpolation.hpp>

namespace grit {

struct BufferInterpolation
{
    struct None
    {
        template<etl::linalg::in_vector Vec, typename Float = typename Vec::value_type>
        [[nodiscard]] constexpr auto operator()(Vec buffer, etl::size_t readPos, Float fracPos) -> Float
        {
            etl::ignore_unused(fracPos);
            return buffer(readPos % buffer.size());
        }
    };

    struct Linear
    {
        template<etl::linalg::in_vector Vec, typename Float = typename Vec::value_type>
        [[nodiscard]] constexpr auto operator()(Vec buffer, etl::size_t readPos, Float fracPos) -> Float
        {
            auto const x0 = buffer(readPos % buffer.size());
            auto const x1 = buffer((readPos + 1) % buffer.size());
            return fastLerp(x0, x1, fracPos);
        }
    };

    struct Hermite
    {
        template<etl::linalg::in_vector Vec, typename Float = typename Vec::value_type>
        [[nodiscard]] constexpr auto operator()(Vec buffer, etl::size_t readPos, Float fracPos) -> Float
        {
            auto const pos = readPos + buffer.size();
            auto const xm1 = buffer((pos - 1) % buffer.size());
            auto const x0  = buffer(pos % buffer.size());
            auto const x1  = buffer((pos + 1) % buffer.size());
            auto const x2  = buffer((pos + 2) % buffer.size());
            return hermiteInterpolation(xm1, x0, x1, x2, fracPos);
        }
    };
};

}  // namespace grit
