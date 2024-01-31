#include "decibel.hpp"

#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

#include <etl/concepts.hpp>
#include <etl/type_traits.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    auto const infinity = grit::defaultMinusInfinityDb<Float>;

    assert(grit::approx(grit::toDecibels(Float(0)), infinity));
    assert(grit::approx(grit::toDecibels(Float(0.00000001)), infinity));
    assert(grit::approx(grit::fromDecibels(infinity), Float(0)));

    assert(grit::approx(grit::toDecibels(grit::fromDecibels(Float(0))), Float(0)));
    assert(grit::approx(grit::toDecibels(grit::fromDecibels(Float(-6))), Float(-6)));
    assert(grit::approx(grit::toDecibels(grit::fromDecibels(Float(-12))), Float(-12)));
    return true;
}

auto testDecibel() -> bool;

auto testDecibel() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
