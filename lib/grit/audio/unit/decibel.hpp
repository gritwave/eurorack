#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

#include <cmath>

namespace grit {

template<etl::floating_point Float>
constexpr auto default_minus_infinity_db = static_cast<Float>(-100);

template<etl::floating_point Float>
auto from_decibels(Float decibels, Float minus_infinity_db = default_minus_infinity_db<Float>) -> Float
{
    return decibels > minus_infinity_db ? etl::pow(Float(10.0), decibels * Float(0.05)) : Float();
}

template<etl::floating_point Float>
auto to_decibels(Float gain, Float minus_infinity_db = default_minus_infinity_db<Float>) -> Float
{
    return gain > Float() ? std::fmax(minus_infinity_db, static_cast<Float>(std::log10(gain)) * Float(20.0))
                          : minus_infinity_db;
}

}  // namespace grit
