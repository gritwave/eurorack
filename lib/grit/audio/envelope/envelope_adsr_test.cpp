#include "envelope_adsr.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEMPLATE_TEST_CASE("audio/envelope: EnvelopeADSR", "", double)
{
    using Float = TestType;

    // TODO: Check why the error with float32 is so high
    static constexpr auto tolerance = sizeof(Float) == 4 ? 1e-2 : 1e-6;

    auto const sampleRate = GENERATE(Float(24000), Float(48000), Float(96000));
    auto const attack     = GENERATE(as<grit::Milliseconds<Float>>{}, Float(125), Float(1000));
    auto const sustain    = GENERATE(Float(0.75), Float(0.9));

    CAPTURE(sampleRate);
    CAPTURE(attack.count());
    CAPTURE(sustain);

    auto const decay   = attack;
    auto const release = attack;

    auto adsr = grit::EnvelopeADSR<Float>{};
    adsr.setSampleRate(sampleRate);
    adsr.setParameter({
        .attack  = attack,
        .decay   = decay,
        .sustain = sustain,
        .release = release,
    });

    auto const advance = [sampleRate](grit::EnvelopeADSR<Float>& env, grit::Seconds<Float> seconds) {
        auto const count = static_cast<int>(seconds.count() * sampleRate);
        for (auto i{0}; i < count - 1; ++i) {
            [[maybe_unused]] auto out = env();
        }
        return env();
    };

    for (auto i{0}; i < 100; ++i) {
        CAPTURE(i);
        REQUIRE_THAT(adsr(), Catch::Matchers::WithinAbs(Float(0), tolerance));
    }

    adsr.gate(true);
    REQUIRE_THAT(advance(adsr, attack), Catch::Matchers::WithinAbs(Float(1), tolerance));  // attack
    REQUIRE_THAT(advance(adsr, decay), Catch::Matchers::WithinAbs(sustain, tolerance));    // decay -> sustain
    REQUIRE_THAT(advance(adsr, decay), Catch::Matchers::WithinAbs(sustain, tolerance));    // sustain

    adsr.gate(false);
    REQUIRE_THAT(advance(adsr, release), Catch::Matchers::WithinAbs(Float(0), tolerance));  // release
}
