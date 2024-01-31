#include "white_noise.hpp"

#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

#include <etl/concepts.hpp>

template<etl::floating_point Float>
static auto test() -> bool
{
    auto proc = grit::WhiteNoise<Float>{42};

    proc.setGain(Float(0.5));
    assert(grit::approx(proc.getGain(), Float(0.5)));

    for (auto i{0}; i < 1'000; ++i) {
        assert(proc.processSample() >= Float(-1.0 * 0.5));
        assert(proc.processSample() <= Float(+1.0 * 0.5));
    }
    return true;
}

auto testWhiteNoise() -> bool;

auto testWhiteNoise() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
