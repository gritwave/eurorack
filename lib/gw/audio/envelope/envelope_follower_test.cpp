#include "envelope_follower.hpp"

#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

template<etl::floating_point Float>
[[nodiscard]] static auto test() noexcept -> bool
{
    auto follower = gw::EnvelopeFollower<Float>{};
    follower.prepare(Float(44'100));
    assert(gw::approx(follower.processSample(Float(0)), Float(0)));
    assert(gw::approx(follower.processSample(Float(0)), Float(0)));

    auto const x1 = follower.processSample(Float(0.25));
    assert(x1 > Float(0));
    assert(x1 < Float(0.25));

    auto const x2 = follower.processSample(Float(0.25));
    assert(x2 > x1);
    assert(x2 < Float(0.25));

    auto const x3 = follower.processSample(Float(x1));
    assert(x3 < x2);
    assert(x3 < Float(0.25));
    assert(x3 > Float(x1));

    follower.reset();
    follower.setParameter({gw::Milliseconds<Float>{12}});
    assert(gw::approx(follower.processSample(Float(0)), Float(0)));

    auto const y1 = follower.processSample(Float(0.25));
    assert(y1 > Float(0));
    assert(y1 < Float(0.25));
    assert(y1 > x1);

    auto const y2 = follower.processSample(Float(0.25));
    assert(y2 > y1);
    assert(y2 < Float(0.25));
    assert(y2 > x2);

    auto const y3 = follower.processSample(Float(y1));
    assert(y3 < y2);
    assert(y3 < Float(0.25));
    assert(y3 > Float(y1));

    return true;
}

auto test_envelope_follower() -> bool;

auto test_envelope_follower() -> bool
{
    assert((test<float>()));
    assert((test<double>()));
    return true;
}
