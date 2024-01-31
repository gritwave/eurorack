#pragma once

#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float, typename Function = Float (*)(Float)>
struct WaveShaper
{
    explicit WaveShaper(Function function) : _function{function} {}

    auto setFunction(Function function) -> void { _function = function; }

    [[nodiscard]] auto processSample(Float input) const noexcept -> Float { return _function(input); }

private:
    Function _function;
};

}  // namespace grit
