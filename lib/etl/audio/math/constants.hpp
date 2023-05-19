#pragma once

namespace etl::audio::numbers
{

template<typename FloatType>
constexpr auto pi = static_cast<FloatType>(3.141592653589793);

template<typename FloatType>
constexpr auto twoPi = pi<FloatType> * static_cast<FloatType>(2);

}  // namespace etl::audio::numbers
