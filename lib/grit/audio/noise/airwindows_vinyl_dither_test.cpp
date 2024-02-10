#include "airwindows_vinyl_dither.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/audio/noise: AirWindowsVinylDither", "", float, double)
{
    using Float = TestType;

    auto proc = grit::AirWindowsVinylDither<Float>{42};
    proc.reset();
    REQUIRE(proc(Float(0.0)) == Catch::Approx(Float(0.0)));

    proc.setDeRez(Float(0.5));
    REQUIRE(proc.getDeRez() == Catch::Approx(Float(0.5)));
    REQUIRE(proc(Float(0.125)) != Float(0.0));
    REQUIRE(proc(Float(0.111)) != Float(0.0));
    REQUIRE(proc(Float(0.113)) != Float(0.0));
}
