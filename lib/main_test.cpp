#undef NDEBUG
#include <cassert>

extern auto test_decibel() -> bool;
extern auto test_fft() -> bool;
extern auto test_ilog2() -> bool;
extern auto test_ipow() -> bool;

auto main() -> int
{
    assert(test_decibel());
    assert(test_fft());
    assert(test_ilog2());
    assert(test_ipow());
    return 0;
}
