#include "decibel.hpp"

#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

#include <etl/concepts.hpp>
#include <etl/type_traits.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    auto const infinity = grit::default_minus_infinity_db<Float>;

    assert(grit::approx(grit::to_decibels(Float(0)), infinity));
    assert(grit::approx(grit::to_decibels(Float(0.00000001)), infinity));
    assert(grit::approx(grit::from_decibels(infinity), Float(0)));

    assert(grit::approx(grit::to_decibels(grit::from_decibels(Float(0))), Float(0)));
    assert(grit::approx(grit::to_decibels(grit::from_decibels(Float(-6))), Float(-6)));
    assert(grit::approx(grit::to_decibels(grit::from_decibels(Float(-12))), Float(-12)));
    return true;
}

auto test_decibel() -> bool;

auto test_decibel() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
