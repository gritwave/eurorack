#pragma once

#include <cmath>

namespace mc
{

template<typename T>
constexpr auto DefaultMinusInfinitydB = static_cast<T>(-100);

template<typename T>
auto decibelsToGain(T decibels, T minusInfinityDb = DefaultMinusInfinitydB<T>) -> T
{
    return decibels > minusInfinityDb ? std::pow(T(10.0), decibels * T(0.05)) : T();
}

template<typename T>
auto gainToDecibels(T gain, T minusInfinityDb = DefaultMinusInfinitydB<T>) -> T
{
    return gain > T() ? std::fmax(minusInfinityDb, static_cast<T>(std::log10(gain)) * T(20.0)) : minusInfinityDb;
}

}  // namespace mc
