#include "static_delay_line.hpp"

#include <gw/math/buffer_interpolation.hpp>
#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float, typename Interpolator>
[[nodiscard]] static auto test() noexcept -> bool
{
    auto delay = gw::StaticDelayLine<Float, 64, Interpolator>{};

    delay.setDelay(Float(1.0));
    assert(gw::approx(delay.popSample(), Float(0.0)));
    delay.pushSample(Float(1.0));
    assert(gw::approx(delay.popSample(), Float(1.0)));

    delay.pushSample(Float(1.0));
    delay.reset();
    assert(gw::approx(delay.popSample(), Float(0.0)));

    return true;
}

template<etl::floating_point Float>
[[nodiscard]] static auto test_interpolators() noexcept -> bool
{
    assert((test<Float, gw::BufferInterpolation::None>()));
    assert((test<Float, gw::BufferInterpolation::Linear>()));
    assert((test<Float, gw::BufferInterpolation::Hermite>()));
    return true;
}

auto test_static_delay_line() -> bool;

auto test_static_delay_line() -> bool
{
    assert((test_interpolators<float>()));
    assert((test_interpolators<double>()));
    return true;
}
