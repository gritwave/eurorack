#pragma once

#include <cmath>

namespace mc
{

// https://github.com/fdeste/ADSR
// http://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code
struct ADSR
{
    ADSR() noexcept;

    auto setAttack(float rate) noexcept -> void;
    auto setDecay(float rate) noexcept -> void;
    auto setSustain(float level) noexcept -> void;
    auto setRelease(float rate) noexcept -> void;

    auto setTargetRatioA(float ratio) noexcept -> void;
    auto setTargetRatioDR(float ratio) noexcept -> void;

    auto reset() noexcept -> void;
    auto gate(bool isOn) noexcept -> void;
    [[nodiscard]] auto processSample() noexcept -> float;

private:
    enum State
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    static float calcCoef(float rate, float targetRatio)
    {
        return std::exp(-std::log((1.0F + targetRatio) / targetRatio) / rate);
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
    float _targetRatioDR{};
};

inline ADSR::ADSR() noexcept
{
    reset();

    setAttack(0.0F);
    setDecay(0.0F);
    setRelease(0.0F);
    setSustain(1.0F);
    setTargetRatioA(0.3F);
    setTargetRatioDR(0.0001F);
}

inline auto ADSR::setAttack(float rate) noexcept -> void
{
    _attackRate = rate;
    _attackCoef = calcCoef(rate, _targetRatioA);
    _attackBase = (1.0F + _targetRatioA) * (1.0F - _attackCoef);
}

inline auto ADSR::setDecay(float rate) noexcept -> void
{
    _decayRate = rate;
    _decayCoef = calcCoef(rate, _targetRatioDR);
    _decayBase = (_sustainLevel - _targetRatioDR) * (1.0F - _decayCoef);
}

inline auto ADSR::setSustain(float level) noexcept -> void
{
    _sustainLevel = level;
    _decayBase    = (_sustainLevel - _targetRatioDR) * (1.0F - _decayCoef);
}

inline auto ADSR::setRelease(float rate) noexcept -> void
{
    _releaseRate = rate;
    _releaseCoef = calcCoef(rate, _targetRatioDR);
    _releaseBase = -_targetRatioDR * (1.0F - _releaseCoef);
}

inline auto ADSR::setTargetRatioA(float ratio) noexcept -> void
{
    if (ratio < 0.000000001) ratio = 0.000000001;  // -180 dB
    _targetRatioA = ratio;
    _attackBase   = (1.0 + _targetRatioA) * (1.0 - _attackCoef);
}

inline auto ADSR::setTargetRatioDR(float ratio) noexcept -> void
{
    if (ratio < 0.000000001) ratio = 0.000000001;  // -180 dB
    _targetRatioDR = ratio;
    _decayBase     = (_sustainLevel - _targetRatioDR) * (1.0 - _decayCoef);
    _releaseBase   = -_targetRatioDR * (1.0 - _releaseCoef);
}

inline auto ADSR::gate(bool isOn) noexcept -> void
{
    if (isOn) { _state = State::Attack; }
    else if (_state != State::Idle) { _state = State::Release; }
}

inline auto ADSR::reset() noexcept -> void
{
    _state  = State::Idle;
    _output = 0.0;
}

inline auto ADSR::processSample() noexcept -> float
{
    switch (_state)
    {
        case State::Idle:
        {
            break;
        }
        case State::Attack:
        {
            _output = _attackBase + _output * _attackCoef;
            if (_output >= 1.0)
            {
                _output = 1.0;
                _state  = State::Decay;
            }
            break;
        }
        case State::Decay:
        {
            _output = _decayBase + _output * _decayCoef;
            if (_output <= _sustainLevel)
            {
                _output = _sustainLevel;
                _state  = State::Sustain;
            }
            break;
        }
        case State::Sustain:
        {
            break;
        }
        case State::Release:
        {
            _output = _releaseBase + _output * _releaseCoef;
            if (_output <= 0.0)
            {
                _output = 0.0;
                _state  = State::Idle;
            }
            break;
        }
    }

    return _output;
}

}  // namespace mc
