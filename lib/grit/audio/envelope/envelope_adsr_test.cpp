#include "envelope_adsr.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEMPLATE_TEST_CASE("audio/envelope: EnvelopeADSR", "", float, double)
{
    using Float = TestType;

    // TODO: Check why the error with float32 is so high
    static constexpr auto tolerance = sizeof(Float) == 4 ? 1e-2 : 1e-6;

    auto const advance = [](auto& env, auto count) {
        for (auto i{0}; i < count - 1; ++i) {
            [[maybe_unused]] auto out = env();
        }
        return env();
    };

    auto const attack  = GENERATE(2000, 44'100, 96'000);
    auto const sustain = GENERATE(Float(0.75), Float(0.9));

    auto const decay   = attack / 4;
    auto const release = attack / 2;

    auto adsr = grit::EnvelopeADSR<Float>{};
    adsr.setAttack(Float(attack));
    adsr.setDecay(Float(decay));
    adsr.setSustain(sustain);
    adsr.setRelease(Float(release));

    for (auto i{0}; i < 100; ++i) {
        CAPTURE(i);
        REQUIRE_THAT(adsr(), Catch::Matchers::WithinAbs(Float(0), tolerance));
    }

    adsr.gate(true);
    REQUIRE_THAT(advance(adsr, attack), Catch::Matchers::WithinAbs(Float(1), tolerance));    // attack
    REQUIRE_THAT(advance(adsr, decay), Catch::Matchers::WithinAbs(sustain, tolerance));      // decay -> sustain
    REQUIRE_THAT(advance(adsr, decay * 2), Catch::Matchers::WithinAbs(sustain, tolerance));  // sustain

    adsr.gate(false);
    REQUIRE_THAT(advance(adsr, release), Catch::Matchers::WithinAbs(Float(0), tolerance));  // release
}
