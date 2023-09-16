#undef NDEBUG
#include <cassert>

extern auto test_decibel() -> bool;
extern auto test_fft() -> bool;
extern auto test_ilog2() -> bool;
extern auto test_ipow() -> bool;
extern auto test_no_dither() -> bool;
extern auto test_note() -> bool;
extern auto test_rectangle_dither() -> bool;
extern auto test_static_delay_line() -> bool;
extern auto test_triangle_dither() -> bool;
extern auto test_white_noise() -> bool;

auto main() -> int
{
    assert(test_decibel());
    assert(test_fft());
    assert(test_ilog2());
    assert(test_ipow());
    assert(test_no_dither());
    assert(test_note());
    assert(test_rectangle_dither());
    assert(test_static_delay_line());
    assert(test_triangle_dither());
    assert(test_white_noise());
    return 0;
}
