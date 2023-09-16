#include "note.hpp"

#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    assert(gw::approx(gw::noteToHertz(Float(57)), Float(220)));
    assert(gw::approx(gw::noteToHertz(Float(69)), Float(440)));
    assert(gw::approx(gw::noteToHertz(Float(81)), Float(880)));

    return true;
}

auto test_note() -> bool;

auto test_note() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
