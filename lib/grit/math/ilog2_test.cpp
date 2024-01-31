#include "ilog2.hpp"

#include <grit/testing/assert.hpp>

#include <etl/type_traits.hpp>

template<etl::integral Int>
static auto test() -> bool
{
    static_assert(etl::same_as<decltype(grit::ilog2(etl::declval<Int>())), Int>);

    assert(grit::ilog2(Int(1)) == Int(0));
    assert(grit::ilog2(Int(2)) == Int(1));
    assert(grit::ilog2(Int(4)) == Int(2));
    assert(grit::ilog2(Int(8)) == Int(3));
    assert(grit::ilog2(Int(16)) == Int(4));
    assert(grit::ilog2(Int(32)) == Int(5));
    assert(grit::ilog2(Int(64)) == Int(6));

    if constexpr (sizeof(Int) > 1) {
        assert(grit::ilog2(Int(128)) == Int(7));
        assert(grit::ilog2(Int(256)) == Int(8));
        assert(grit::ilog2(Int(512)) == Int(9));
        assert(grit::ilog2(Int(1024)) == Int(10));
        assert(grit::ilog2(Int(2048)) == Int(11));
        assert(grit::ilog2(Int(4096)) == Int(12));
        assert(grit::ilog2(Int(8192)) == Int(13));
    }

    return true;
}

auto testIlog2() -> bool;

auto testIlog2() -> bool
{
    assert((test<signed char>()));
    assert((test<signed short>()));
    assert((test<signed int>()));
    assert((test<signed long>()));
    assert((test<signed long long>()));

    assert((test<unsigned char>()));
    assert((test<unsigned short>()));
    assert((test<unsigned int>()));
    assert((test<unsigned long>()));
    assert((test<unsigned long long>()));

    return true;
}