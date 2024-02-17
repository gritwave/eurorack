#include "note.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("audio/music: noteToHertz", "", float, double)
{
    using Float = TestType;

    REQUIRE(grit::noteToHertz(Float(57)) == Catch::Approx(220.0));
    REQUIRE(grit::noteToHertz(Float(69)) == Catch::Approx(440.0));
    REQUIRE(grit::noteToHertz(Float(81)) == Catch::Approx(880.0));
}
