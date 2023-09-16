#include "fft.hpp"

#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

template<etl::floating_point Float, etl::size_t Size>
auto test_static_fft_plan() -> bool
{
    using Complex = etl::complex<Float>;
    using Plan    = gw::fft::static_fft_plan<Complex, Size>;

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

auto test_fft() -> bool;

auto test_fft() -> bool
{
    assert((test_static_fft_plan<float, 4>()));
    assert((test_static_fft_plan<float, 8>()));
    assert((test_static_fft_plan<float, 16>()));
    assert((test_static_fft_plan<float, 32>()));
    assert((test_static_fft_plan<float, 64>()));
    assert((test_static_fft_plan<float, 128>()));
    assert((test_static_fft_plan<float, 256>()));
    assert((test_static_fft_plan<float, 512>()));
    assert((test_static_fft_plan<float, 1024>()));

    assert((test_static_fft_plan<double, 4>()));
    assert((test_static_fft_plan<double, 8>()));
    assert((test_static_fft_plan<double, 16>()));
    assert((test_static_fft_plan<double, 32>()));
    assert((test_static_fft_plan<double, 64>()));
    assert((test_static_fft_plan<double, 128>()));
    assert((test_static_fft_plan<double, 256>()));
    assert((test_static_fft_plan<double, 512>()));
    assert((test_static_fft_plan<double, 1024>()));
    return true;
}
