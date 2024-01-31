#include "static_delay_line.hpp"

#include <grit/math/buffer_interpolation.hpp>
#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float, typename Interpolator>
[[nodiscard]] static auto test() -> bool
{
    auto delay = grit::StaticDelayLine<Float, 64, Interpolator>{};

    delay.setDelay(Float(1.0));
    assert(grit::approx(delay.popSample(), Float(0.0)));
    delay.pushSample(Float(1.0));
    assert(grit::approx(delay.popSample(), Float(1.0)));

    delay.pushSample(Float(1.0));
    delay.reset();
    assert(grit::approx(delay.popSample(), Float(0.0)));

    return true;
}

template<etl::floating_point Float>
[[nodiscard]] static auto testInterpolators() -> bool
{
    assert((test<Float, grit::BufferInterpolation::None>()));
    assert((test<Float, grit::BufferInterpolation::Linear>()));
    assert((test<Float, grit::BufferInterpolation::Hermite>()));
    return true;
}

auto testStaticDelayLine() -> bool;

auto testStaticDelayLine() -> bool
{
    assert((testInterpolators<float>()));
    assert((testInterpolators<double>()));
    return true;
}
