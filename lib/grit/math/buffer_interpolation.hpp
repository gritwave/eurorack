#pragma once

#include <grit/core/mdspan.hpp>
#include <grit/math/fast_lerp.hpp>
#include <grit/math/hermite_interpolation.hpp>

namespace grit {

struct buffer_interpolation
{
    struct none
    {
        template<etl::linalg::in_vector Vec, typename Float = typename Vec::value_type>
        [[nodiscard]] constexpr auto operator()(Vec buffer, etl::size_t read_pos, Float frac_pos) -> Float
        {
            etl::ignore_unused(frac_pos);
            return buffer(read_pos % buffer.size());
        }
    };

    struct linear
    {
        template<etl::linalg::in_vector Vec, typename Float = typename Vec::value_type>
        [[nodiscard]] constexpr auto operator()(Vec buffer, etl::size_t read_pos, Float frac_pos) -> Float
        {
            auto const x0 = buffer(read_pos % buffer.size());
            auto const x1 = buffer((read_pos + 1) % buffer.size());
            return fast_lerp(x0, x1, frac_pos);
        }
    };

    struct hermite
    {
        template<etl::linalg::in_vector Vec, typename Float = typename Vec::value_type>
        [[nodiscard]] constexpr auto operator()(Vec buffer, etl::size_t read_pos, Float frac_pos) -> Float
        {
            auto const pos = read_pos + buffer.size();
            auto const xm1 = buffer((pos - 1) % buffer.size());
            auto const x0  = buffer(pos % buffer.size());
            auto const x1  = buffer((pos + 1) % buffer.size());
            auto const x2  = buffer((pos + 2) % buffer.size());
            return hermite_interpolation(xm1, x0, x1, x2, frac_pos);
        }
    };
};

}  // namespace grit
