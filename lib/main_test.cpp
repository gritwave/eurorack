#undef NDEBUG
#include <cassert>

extern auto test_fft() -> bool;
extern auto test_ipow() -> bool;

auto main() -> int
{
    assert(test_ipow());
    assert(test_fft());
    return 0;
}
