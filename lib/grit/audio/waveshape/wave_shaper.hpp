#pragma once

#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point SampleType, typename Function = SampleType (*)(SampleType)>
struct wave_shaper
{
    explicit wave_shaper(Function function) : _function{function} {}

    auto set_function(Function function) -> void { _function = function; }

    [[nodiscard]] auto process_sample(SampleType input) const noexcept -> SampleType { return _function(input); }

private:
    Function _function;
};

}  // namespace grit
