#pragma once

#include <etl/cmath.hpp>

namespace grit {

// https://github.com/fdeste/ADSR
// http://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code
struct ADSR
{
    ADSR();

    auto setAttack(float rate) -> void;
    auto setDecay(float rate) -> void;
    auto setSustain(float level) -> void;
    auto setRelease(float rate) -> void;

    auto setTargetRatioA(float ratio) -> void;
    auto setTargetRatioDr(float ratio) -> void;

    auto reset() -> void;
    auto gate(bool isOn) -> void;
    [[nodiscard]] auto processSample() -> float;

private:
    enum State
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    static auto calcCoef(float rate, float targetRatio) -> float
    {
        return etl::exp(-etl::log((1.0F + targetRatio) / targetRatio) / rate);
    }

    State _state{State::Idle};
    float _output{};

    float _attackRate{};
    float _attackCoef{};
    float _attackBase{};

    float _decayRate{};
    float _decayCoef{};
    float _decayBase{};

    float _sustainLevel{};

    float _releaseRate{};
    float _releaseCoef{};
    float _releaseBase{};

    float _targetRatioA{};
    float _targetRatioDr{};
};

inline ADSR::ADSR()
{
    reset();

    setAttack(0.0F);
    setDecay(0.0F);
    setRelease(0.0F);
    setSustain(1.0F);
    setTargetRatioA(0.3F);
    setTargetRatioDr(0.0001F);
}

inline auto ADSR::setAttack(float rate) -> void
{
    _attackRate = rate;
    _attackCoef = calcCoef(rate, _targetRatioA);
    _attackBase = (1.0F + _targetRatioA) * (1.0F - _attackCoef);
}

inline auto ADSR::setDecay(float rate) -> void
{
    _decayRate = rate;
    _decayCoef = calcCoef(rate, _targetRatioDr);
    _decayBase = (_sustainLevel - _targetRatioDr) * (1.0F - _decayCoef);
}

inline auto ADSR::setSustain(float level) -> void
{
    _sustainLevel = level;
    _decayBase    = (_sustainLevel - _targetRatioDr) * (1.0F - _decayCoef);
}

inline auto ADSR::setRelease(float rate) -> void
{
    _releaseRate = rate;
    _releaseCoef = calcCoef(rate, _targetRatioDr);
    _releaseBase = -_targetRatioDr * (1.0F - _releaseCoef);
}

inline auto ADSR::setTargetRatioA(float ratio) -> void
{
    if (ratio < 0.000000001) {
        ratio = 0.000000001;  // -180 dB
    }
    _targetRatioA = ratio;
    _attackBase   = (1.0 + _targetRatioA) * (1.0 - _attackCoef);
}

inline auto ADSR::setTargetRatioDr(float ratio) -> void
{
    if (ratio < 0.000000001) {
        ratio = 0.000000001;  // -180 dB
    }
    _targetRatioDr = ratio;
    _decayBase     = (_sustainLevel - _targetRatioDr) * (1.0 - _decayCoef);
    _releaseBase   = -_targetRatioDr * (1.0 - _releaseCoef);
}

inline auto ADSR::gate(bool isOn) -> void
{
    if (isOn) {
        _state = State::Attack;
    } else if (_state != State::Idle) {
        _state = State::Release;
    }
}

inline auto ADSR::reset() -> void
{
    _state  = State::Idle;
    _output = 0.0;
}

inline auto ADSR::processSample() -> float
{
    switch (_state) {
        case State::Idle: {
            break;
        }
        case State::Attack: {
            _output = _attackBase + _output * _attackCoef;
            if (_output >= 1.0) {
                _output = 1.0;
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
            if (_output <= 0.0) {
                _output = 0.0;
                _state  = State::Idle;
            }
            break;
        }
    }

    return _output;
}

}  // namespace grit
