#include "static_delay_line.hpp"

#include <grit/math/buffer_interpolation.hpp>
#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float, typename Interpolator>
[[nodiscard]] static auto test() noexcept -> bool
{
    auto delay = grit::static_delay_line<Float, 64, Interpolator>{};

    delay.set_delay(Float(1.0));
    assert(grit::approx(delay.pop_sample(), Float(0.0)));
    delay.push_sample(Float(1.0));
    assert(grit::approx(delay.pop_sample(), Float(1.0)));

    delay.push_sample(Float(1.0));
    delay.reset();
    assert(grit::approx(delay.pop_sample(), Float(0.0)));

    return true;
}

template<etl::floating_point Float>
[[nodiscard]] static auto test_interpolators() noexcept -> bool
{
    assert((test<Float, grit::buffer_interpolation::none>()));
    assert((test<Float, grit::buffer_interpolation::linear>()));
    assert((test<Float, grit::buffer_interpolation::hermite>()));
    return true;
}

auto test_static_delay_line() -> bool;

auto test_static_delay_line() -> bool
{
    assert((test_interpolators<float>()));
    assert((test_interpolators<double>()));
    return true;
}
