#pragma once

#include <etl/concepts.hpp>

#include <cmath>

namespace gw
{

template<etl::floating_point Float>
constexpr auto DefaultMinusInfinitydB = static_cast<Float>(-100);

template<etl::floating_point Float>
auto decibelsToGain(Float decibels, Float minusInfinityDb = DefaultMinusInfinitydB<Float>) -> Float
{
    return decibels > minusInfinityDb ? std::pow(Float(10.0), decibels * Float(0.05)) : Float();
}

template<etl::floating_point Float>
auto gainToDecibels(Float gain, Float minusInfinityDb = DefaultMinusInfinitydB<Float>) -> Float
{
    return gain > Float() ? std::fmax(minusInfinityDb, static_cast<Float>(std::log10(gain)) * Float(20.0))
                          : minusInfinityDb;
}

}  // namespace gw
