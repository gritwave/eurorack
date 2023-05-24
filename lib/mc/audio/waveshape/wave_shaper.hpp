#pragma once

#include <etl/concepts.hpp>

#include <cmath>

namespace mc::audio
{

template<etl::floating_point FloatType, typename Function = FloatType (*)(FloatType)>
struct WaveShaper
{
    explicit WaveShaper(Function function) : _function{function} {}

    auto setFunction(Function function) -> void { _function = function; }

    [[nodiscard]] auto processSample(FloatType inputSample) const noexcept -> FloatType
    {
        return _function(inputSample);
    }

private:
    Function _function;
};

}  // namespace mc::audio
