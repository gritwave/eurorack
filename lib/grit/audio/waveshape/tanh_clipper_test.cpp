#include "tanh_clipper.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/audio/waveshape: TanhClipper", "", float, double)
{
    using Float = TestType;

    auto shaper = grit::TanhClipper<Float>{};
    STATIC_REQUIRE(etl::is_empty_v<grit::TanhClipper<Float>>);

    REQUIRE(shaper(Float(-2.0)) == Catch::Approx(-0.9640275801));
    REQUIRE(shaper(Float(-1.5)) == Catch::Approx(-0.9051482536));
    REQUIRE(shaper(Float(-1.0)) == Catch::Approx(-0.7615941559));
    REQUIRE(shaper(Float(-0.5)) == Catch::Approx(-0.4621171572));
    REQUIRE(shaper(Float(+0.0)) == Catch::Approx(+0.0));
    REQUIRE(shaper(Float(+0.5)) == Catch::Approx(+0.4621171572));
    REQUIRE(shaper(Float(+1.0)) == Catch::Approx(+0.7615941559));
}

TEMPLATE_TEST_CASE("grit/audio/waveshape: TanhClipperADAA1", "", float, double)
{
    using Float = TestType;

    auto shaper = grit::TanhClipperADAA1<Float>{};
    REQUIRE(shaper(Float(-2.0)) < Float(0));
    REQUIRE(shaper(Float(-2.0)) == Catch::Approx(-0.9640275801));

    REQUIRE(shaper(Float(-1.0)) < Float(0));
    REQUIRE(shaper(Float(-1.0)) == Catch::Approx(-0.7615941559));
}
