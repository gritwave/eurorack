#pragma once

#include <etl/concepts.hpp>

#include <cmath>

namespace gw
{

template<etl::floating_point SampleType, typename Function = SampleType (*)(SampleType)>
struct WaveShaper
{
    explicit WaveShaper(Function function) : _function{function} {}

    auto setFunction(Function function) -> void { _function = function; }

    [[nodiscard]] auto processSample(SampleType input) const noexcept -> SampleType { return _function(input); }

private:
    Function _function;
};

}  // namespace gw
