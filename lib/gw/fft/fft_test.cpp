#include "fft.hpp"

#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

template<etl::floating_point Float, etl::size_t Size>
auto test_twiddles() -> bool
{
    auto const wf = gw::fft::make_twiddles_r2<Float, Size>(gw::fft::direction::forward);
    auto const wb = gw::fft::make_twiddles_r2<Float, Size>(gw::fft::direction::backward);
    assert(wf.size() == wb.size());

    for (auto i{0U}; i < wf.size(); ++i) {
        assert(gw::approx(wf[i], etl::conj(wb[i])));
    }

    return true;
}

template<typename Plan>
auto test_static_fft_plan() -> bool
{
    using Complex = typename Plan::value_type;
    using Float   = typename Complex::value_type;

    auto plan  = Plan{};
    auto x_buf = etl::array<Complex, Plan::size()>{};
    auto x     = etl::mdspan{x_buf.data(), etl::extents{x_buf.size()}};
    x(0)       = etl::complex{Float(1), Float(0)};

    plan(x, gw::fft::direction::forward);
    for (auto val : x_buf) {
        assert(gw::approx(val, etl::complex{Float(1)}));
    }

    plan(x, gw::fft::direction::backward);
    assert(gw::approx(x(0), etl::complex{Float(1) * Float(Plan::size())}));

    for (auto val : etl::span{x_buf}.last(Plan::size() - 1)) {
        assert(gw::approx(val, etl::complex{Float(0)}));
    }

    return true;
}

template<etl::floating_point Float, etl::size_t Size>
auto test_plan() -> bool
{
    test_static_fft_plan<gw::fft::static_fft_plan<etl::complex<Float>, Size>>();
    test_static_fft_plan<gw::fft::static_fft_plan_v2<etl::complex<Float>, Size>>();
    return true;
}

template<etl::size_t Size>
static auto test_size() -> bool
{
    assert((test_twiddles<float, Size>()));
    assert((test_twiddles<double, Size>()));
    assert((test_plan<float, Size>()));
    assert((test_plan<double, Size>()));
    return true;
}

auto test_fft() -> bool;

auto test_fft() -> bool
{
    assert((test_size<4>()));
    assert((test_size<8>()));
    assert((test_size<16>()));
    assert((test_size<32>()));
    assert((test_size<64>()));
    assert((test_size<128>()));
    assert((test_size<256>()));
    assert((test_size<512>()));
    assert((test_size<1024>()));

    return true;
}
