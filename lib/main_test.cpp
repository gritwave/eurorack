#undef NDEBUG
#include <cassert>

#include <grit/core/benchmark.hpp>
#include <grit/eurorack/hades.hpp>
#include <grit/math/buffer_interpolation.hpp>
#include <grit/math/linear_interpolation.hpp>
#include <grit/math/range.hpp>

extern auto testDecibel() -> bool;
extern auto testEnvelopeFollower() -> bool;
extern auto testFft() -> bool;
extern auto testIlog2() -> bool;
extern auto testIpow() -> bool;
extern auto testNoDither() -> bool;
extern auto testNote() -> bool;
extern auto testRectangleDither() -> bool;
extern auto testStaticDelayLine() -> bool;
extern auto testTriangleDither() -> bool;
extern auto testWhiteNoise() -> bool;

auto main() -> int
{
    assert(testDecibel());
    assert(testEnvelopeFollower());
    assert(testFft());
    assert(testIlog2());
    assert(testIpow());
    assert(testNoDither());
    assert(testNote());
    assert(testRectangleDither());
    assert(testStaticDelayLine());
    assert(testTriangleDither());
    assert(testWhiteNoise());
    return 0;
}
