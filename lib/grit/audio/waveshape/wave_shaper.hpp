#pragma once

#include <etl/concepts.hpp>
#include <etl/type_traits.hpp>

namespace grit {

template<etl::floating_point Float, typename Function = Float (*)(Float)>
struct WaveShaper
{
    using value_type = Float;

    WaveShaper()
        requires(etl::is_empty_v<Function>)
    = default;

    explicit WaveShaper(Function function) : _function{function} {}

    auto setFunction(Function function) -> void { _function = function; }

    static auto reset() -> void {}

    [[nodiscard]] auto operator()(Float input) const -> Float { return _function(input); }

private:
    [[no_unique_address]] Function _function;
};

}  // namespace grit
