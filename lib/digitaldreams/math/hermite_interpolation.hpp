#pragma once

namespace digitaldreams::audio
{

template<typename T>
auto hermite4(T pos, T xm1, T x0, T x1, T x2) noexcept -> T
{
    auto const slope0 = (x1 - xm1) * static_cast<T>(0.5);
    auto const slope1 = (x2 - x0) * static_cast<T>(0.5);

    auto const v    = x0 - x1;
    auto const w    = slope0 + v;
    auto const a    = w + v + slope1;
    auto const bNeg = w + a;

    auto const stage1 = a * pos - bNeg;
    auto const stage2 = stage1 * pos + slope0;

    return stage2 * pos + x0;
}

}  // namespace digitaldreams::audio
