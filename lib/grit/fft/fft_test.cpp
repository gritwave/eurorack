#include "fft.hpp"

#include <grit/testing/approx.hpp>
#include <grit/testing/assert.hpp>

template<etl::floating_point Float, etl::size_t Size>
auto testTwiddles() -> bool
{
    auto const wf = grit::fft::makeTwiddles<Float, Size>(grit::fft::Direction::Forward);
    auto const wb = grit::fft::makeTwiddles<Float, Size>(grit::fft::Direction::Backward);
    assert(wf.size() == wb.size());

    for (auto i{0U}; i < wf.size(); ++i) {
        assert(grit::approx(wf[i], etl::conj(wb[i])));
    }

    return true;
}

template<typename Plan>
auto testComplexPlan() -> bool
{
    using Complex = typename Plan::value_type;
    using Float   = typename Complex::value_type;

    auto plan = Plan{};
    auto xBuf = etl::array<Complex, Plan::size()>{};
    auto x    = etl::mdspan{xBuf.data(), etl::extents{xBuf.size()}};
    x(0)      = etl::complex{Float(1), Float(0)};

    plan(x, grit::fft::Direction::Forward);
    for (auto const& val : xBuf) {
        assert(grit::approx(val, etl::complex{Float(1)}));
    }

    plan(x, grit::fft::Direction::Backward);
    assert(grit::approx(x(0), etl::complex{Float(1) * Float(Plan::size())}));

    for (auto const& val : etl::span{xBuf}.last(Plan::size() - 1)) {
        assert(grit::approx(val, etl::complex{Float(0)}));
    }

    return true;
}

template<etl::floating_point Float, etl::size_t Size>
auto testPlan() -> bool
{
    testComplexPlan<grit::fft::ComplexPlan<etl::complex<Float>, Size>>();
    testComplexPlan<grit::fft::ComplexPlanV2<etl::complex<Float>, Size>>();
    return true;
}

template<etl::size_t Size>
static auto testSize() -> bool
{
    assert((testTwiddles<float, Size>()));
    assert((testTwiddles<double, Size>()));
    assert((testPlan<float, Size>()));
    assert((testPlan<double, Size>()));
    return true;
}

auto testFft() -> bool;

auto testFft() -> bool
{
    assert((testSize<4>()));
    assert((testSize<8>()));
    assert((testSize<16>()));
    assert((testSize<32>()));
    assert((testSize<64>()));
    assert((testSize<128>()));
    assert((testSize<256>()));
    assert((testSize<512>()));
    assert((testSize<1024>()));

    return true;
}
