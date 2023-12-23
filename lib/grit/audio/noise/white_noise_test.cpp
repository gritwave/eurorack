#include "white_noise.hpp"

#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    auto proc = grit::white_noise<Float>{42};

    proc.set_gain(Float(0.5));
    assert(grit::approx(proc.get_gain(), Float(0.5)));

    for (auto i{0}; i < 1'000; ++i) {
        assert(proc.process_sample() >= Float(-1.0 * 0.5));
        assert(proc.process_sample() <= Float(+1.0 * 0.5));
    }
    return true;
}

auto test_white_noise() -> bool;

auto test_white_noise() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
