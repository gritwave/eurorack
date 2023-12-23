#pragma once

#include <etl/complex.hpp>
#include <etl/concepts.hpp>
#include <etl/limits.hpp>

namespace grit {

template<etl::floating_point Float>
[[nodiscard]] static auto approx(Float x, Float y, int scale = 8) -> bool
{
    auto const margin = etl::numeric_limits<Float>::epsilon() * Float(scale);
    return etl::abs(x - y) <= margin;
}

template<etl::floating_point Float>
[[nodiscard]] static auto approx(etl::complex<Float> x, etl::complex<Float> y, int scale = 8) -> bool
{
    return approx(x.real(), y.real(), scale) and approx(x.imag(), y.imag(), scale);
}

}  // namespace grit
