#include "note.hpp"

#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    assert(grit::approx(grit::noteToHertz(Float(57)), Float(220)));
    assert(grit::approx(grit::noteToHertz(Float(69)), Float(440)));
    assert(grit::approx(grit::noteToHertz(Float(81)), Float(880)));

    return true;
}

auto test_note() -> bool;

auto test_note() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
