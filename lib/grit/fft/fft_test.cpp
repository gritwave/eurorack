#include "fft.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

template<typename Plan>
auto testComplexPlan() -> void
{
    using Complex = typename Plan::value_type;
    using Float   = typename Complex::value_type;

    auto plan = Plan{};
    auto xBuf = etl::array<Complex, Plan::size()>{};
    auto x    = etl::mdspan{xBuf.data(), etl::extents{xBuf.size()}};
    x(0)      = etl::complex{Float(1), Float(0)};

    plan(x, grit::fft::Direction::Forward);
    for (auto const& val : xBuf) {
        REQUIRE(val.real() == Catch::Approx(etl::complex{Float(1)}.real()));
        REQUIRE(val.imag() == Catch::Approx(etl::complex{Float(1)}.imag()));
    }

    plan(x, grit::fft::Direction::Backward);
    REQUIRE(x(0).real() == Catch::Approx(etl::complex{Float(1) * Float(Plan::size())}.real()));
    REQUIRE(x(0).imag() == Catch::Approx(etl::complex{Float(1) * Float(Plan::size())}.imag()));

    for (auto const& val : etl::span{xBuf}.last(Plan::size() - 1)) {
        REQUIRE(val.real() == Catch::Approx(etl::complex{Float(0)}.real()));
        REQUIRE(val.imag() == Catch::Approx(etl::complex{Float(0)}.imag()));
    }
}

TEMPLATE_TEST_CASE("fft: ComplexPlan", "", etl::complex<float>, etl::complex<double>)
{
    testComplexPlan<grit::fft::ComplexPlan<TestType, 64>>();
    testComplexPlan<grit::fft::ComplexPlan<TestType, 128>>();
    testComplexPlan<grit::fft::ComplexPlan<TestType, 256>>();
    testComplexPlan<grit::fft::ComplexPlan<TestType, 512>>();
    testComplexPlan<grit::fft::ComplexPlan<TestType, 1024>>();
}

TEMPLATE_TEST_CASE("fft: ComplexPlanV2", "", etl::complex<float>, etl::complex<double>)
{
    testComplexPlan<grit::fft::ComplexPlanV2<TestType, 64>>();
    testComplexPlan<grit::fft::ComplexPlanV2<TestType, 128>>();
    testComplexPlan<grit::fft::ComplexPlanV2<TestType, 256>>();
    testComplexPlan<grit::fft::ComplexPlanV2<TestType, 512>>();
    testComplexPlan<grit::fft::ComplexPlanV2<TestType, 1024>>();
}
