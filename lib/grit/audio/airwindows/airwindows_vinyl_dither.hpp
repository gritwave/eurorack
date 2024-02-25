#pragma once

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace grit {

/// \ingroup grit-audio-airwindows
template<etl::floating_point Float, typename URNG = etl::xoshiro128plusplus>
struct AirWindowsVinylDither
{
    using SampleType = Float;
    using SeedType   = typename URNG::result_type;

    AirWindowsVinylDither() = default;
    explicit AirWindowsVinylDither(SeedType seed);

    auto setDeRez(Float deRez) -> void;
    [[nodiscard]] auto getDeRez() const -> Float;

    [[nodiscard]] auto operator()(Float x) -> Float;
    auto reset() -> void;

private:
    auto advanceNoise() -> Float;

    Float _deRez{0};
    Float _inScale{0};
    Float _outScale{0};

    URNG _rng{42};
    etl::uniform_real_distribution<Float> _dist{Float(-0.5), Float(0.5)};

    Float _nsOdd{0};
    Float _prev{0};
    etl::array<Float, 16> _ns{};
};

template<etl::floating_point Float, typename URNG>
AirWindowsVinylDither<Float, URNG>::AirWindowsVinylDither(SeedType seed) : _rng{seed}
{}

template<etl::floating_point Float, typename URNG>
auto AirWindowsVinylDither<Float, URNG>::setDeRez(Float deRez) -> void
{
    _deRez = deRez;

    auto scaleFactor = Float(32768.0);
    if (deRez > Float(0)) {
        scaleFactor *= etl::pow(Float(1) - deRez, Float(6));
    }
    if (scaleFactor < Float(0.0001)) {
        scaleFactor = Float(0.0001);
    }

    auto outScale = scaleFactor;
    if (outScale < Float(8)) {
        outScale = Float(8);
    }

    _inScale  = scaleFactor;
    _outScale = Float(1) / outScale;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsVinylDither<Float, URNG>::getDeRez() const -> Float
{
    return _deRez;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsVinylDither<Float, URNG>::operator()(Float x) -> Float
{
    auto const y = x * _inScale;

    auto absSample = advanceNoise();
    absSample += y;

    if (_nsOdd > 0) {
        _nsOdd -= Float(0.97);
    }
    if (_nsOdd < 0) {
        _nsOdd += Float(0.97);
    }

    _nsOdd -= (_nsOdd * _nsOdd * _nsOdd * Float(0.475));
    _nsOdd += _prev;

    absSample += (_nsOdd * Float(0.475));
    auto const floor = etl::floor(absSample);

    _prev = floor - y;
    return floor * _outScale;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsVinylDither<Float, URNG>::reset() -> void
{
    _nsOdd = Float(0);
    _prev  = Float(0);
    etl::fill(_ns.begin(), _ns.end(), Float(0));
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsVinylDither<Float, URNG>::advanceNoise() -> Float
{
    auto absSample = _dist(_rng);
    _ns[0] += absSample;
    _ns[0] *= Float(0.5);
    absSample -= _ns[0];

    for (auto i{1U}; i < _ns.size(); ++i) {
        absSample += _dist(_rng);
        _ns[i] += absSample;
        _ns[i] *= Float(0.5);
        absSample -= _ns[i];
    }

    return absSample;
}

}  // namespace grit
