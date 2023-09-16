#include "decibel.hpp"

#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

#include <etl/concepts.hpp>
#include <etl/type_traits.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    assert(gw::approx(gw::toDecibels(gw::fromDecibels(Float(0))), Float(0)));
    assert(gw::approx(gw::toDecibels(gw::fromDecibels(Float(-6))), Float(-6)));
    assert(gw::approx(gw::toDecibels(gw::fromDecibels(Float(-12))), Float(-12)));
    return true;
}

auto test_decibel() -> bool;

auto test_decibel() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
