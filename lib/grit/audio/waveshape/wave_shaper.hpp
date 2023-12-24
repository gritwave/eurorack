#pragma once

#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float, typename Function = Float (*)(Float)>
struct wave_shaper
{
    explicit wave_shaper(Function function) : _function{function} {}

    auto set_function(Function function) -> void { _function = function; }

    [[nodiscard]] auto process_sample(Float input) const noexcept -> Float { return _function(input); }

private:
    Function _function;
};

}  // namespace grit
