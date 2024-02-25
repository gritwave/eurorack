#pragma once

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// ADSR Envelope Generator
///
/// - https://github.com/fdeste/ADSR
/// - http://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code
///
/// \ingroup grit-audio-envelope
template<etl::floating_point Float>
struct EnvelopeADSR
{
    using SampleType = Float;

    constexpr EnvelopeADSR();

    constexpr auto setAttack(Float rate) -> void;
    constexpr auto setDecay(Float rate) -> void;
    constexpr auto setSustain(Float level) -> void;
    constexpr auto setRelease(Float rate) -> void;

    constexpr auto setTargetRatioA(Float ratio) -> void;
    constexpr auto setTargetRatioDr(Float ratio) -> void;

    constexpr auto gate(bool isOn) -> void;
    constexpr auto reset() -> void;

    [[nodiscard]] constexpr auto operator()() -> Float;

private:
    enum State
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    static constexpr auto calcCoef(Float rate, Float targetRatio) -> Float
    {
        return etl::exp(-etl::log((Float(1) + targetRatio) / targetRatio) / rate);
    }

    State _state{State::Idle};
    Float _output{};

    Float _attackRate{};
    Float _attackCoef{};
    Float _attackBase{};

    Float _decayRate{};
    Float _decayCoef{};
    Float _decayBase{};

    Float _sustainLevel{};

    Float _releaseRate{};
    Float _releaseCoef{};
    Float _releaseBase{};

    Float _targetRatioA{};
    Float _targetRatioDr{};
};

template<etl::floating_point Float>
constexpr EnvelopeADSR<Float>::EnvelopeADSR()
{
    reset();

    setAttack(Float(0));
    setDecay(Float(0));
    setRelease(Float(0));
    setSustain(Float(1));
    setTargetRatioA(Float(0.3));
    setTargetRatioDr(Float(0.0001));
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::setAttack(Float rate) -> void
{
    _attackRate = rate;
    _attackCoef = calcCoef(rate, _targetRatioA);
    _attackBase = (Float(1) + _targetRatioA) * (Float(1) - _attackCoef);
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::setDecay(Float rate) -> void
{
    _decayRate = rate;
    _decayCoef = calcCoef(rate, _targetRatioDr);
    _decayBase = (_sustainLevel - _targetRatioDr) * (Float(1) - _decayCoef);
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::setSustain(Float level) -> void
{
    _sustainLevel = level;
    _decayBase    = (_sustainLevel - _targetRatioDr) * (Float(1) - _decayCoef);
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::setRelease(Float rate) -> void
{
    _releaseRate = rate;
    _releaseCoef = calcCoef(rate, _targetRatioDr);
    _releaseBase = -_targetRatioDr * (Float(1) - _releaseCoef);
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::setTargetRatioA(Float ratio) -> void
{
    _targetRatioA = etl::clamp(ratio, Float(0.000000001), Float(1));
    _attackBase   = (Float(1) + _targetRatioA) * (Float(1) - _attackCoef);
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::setTargetRatioDr(Float ratio) -> void
{
    _targetRatioDr = etl::clamp(ratio, Float(0.000000001), Float(1));
    _decayBase     = (_sustainLevel - _targetRatioDr) * (Float(1) - _decayCoef);
    _releaseBase   = -_targetRatioDr * (Float(1) - _releaseCoef);
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::gate(bool isOn) -> void
{
    if (isOn) {
        _state = State::Attack;
    } else if (_state != State::Idle) {
        _state = State::Release;
    }
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::reset() -> void
{
    _state  = State::Idle;
    _output = Float(0);
}

template<etl::floating_point Float>
constexpr auto EnvelopeADSR<Float>::operator()() -> Float
{
    switch (_state) {
        case State::Idle: {
            break;
        }
        case State::Attack: {
            _output = _attackBase + _output * _attackCoef;
            if (_output >= Float(1)) {
                _output = Float(1);
                _state  = State::Decay;
            }
            break;
        }
        case State::Decay: {
            _output = _decayBase + _output * _decayCoef;
            if (_output <= _sustainLevel) {
                _output = _sustainLevel;
                _state  = State::Sustain;
            }
            break;
        }
        case State::Sustain: {
            break;
        }
        case State::Release: {
            _output = _releaseBase + _output * _releaseCoef;
            if (_output <= Float(0)) {
                _output = Float(0);
                _state  = State::Idle;
            }
            break;
        }
    }

    return _output;
}

}  // namespace grit
