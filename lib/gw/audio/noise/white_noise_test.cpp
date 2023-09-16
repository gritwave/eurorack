#include "white_noise.hpp"

#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    auto proc = gw::WhiteNoise<Float>{42};

    proc.setGain(Float(0.5));
    assert(gw::approx(proc.getGain(), Float(0.5)));

    for (auto i{0}; i < 1'000; ++i) {
        assert(proc.processSample() >= Float(-1.0 * 0.5));
        assert(proc.processSample() <= Float(+1.0 * 0.5));
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
