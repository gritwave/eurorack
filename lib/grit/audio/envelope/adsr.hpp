#pragma once

#include <etl/cmath.hpp>

namespace grit {

// https://github.com/fdeste/ADSR
// http://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code
struct adsr
{
    adsr();

    auto set_attack(float rate) -> void;
    auto set_decay(float rate) -> void;
    auto set_sustain(float level) -> void;
    auto set_release(float rate) -> void;

    auto set_target_ratio_a(float ratio) -> void;
    auto set_target_ratio_dr(float ratio) -> void;

    auto reset() -> void;
    auto gate(bool is_on) -> void;
    [[nodiscard]] auto process_sample() -> float;

private:
    enum state
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    static auto calc_coef(float rate, float target_ratio) -> float
    {
        return etl::exp(-etl::log((1.0F + target_ratio) / target_ratio) / rate);
    }

    state _state{state::Idle};
    float _output{};

    float _attack_rate{};
    float _attack_coef{};
    float _attack_base{};

    float _decay_rate{};
    float _decay_coef{};
    float _decay_base{};

    float _sustain_level{};

    float _release_rate{};
    float _release_coef{};
    float _release_base{};

    float _target_ratio_a{};
    float _target_ratio_dr{};
};

inline adsr::adsr()
{
    reset();

    set_attack(0.0F);
    set_decay(0.0F);
    set_release(0.0F);
    set_sustain(1.0F);
    set_target_ratio_a(0.3F);
    set_target_ratio_dr(0.0001F);
}

inline auto adsr::set_attack(float rate) -> void
{
    _attack_rate = rate;
    _attack_coef = calc_coef(rate, _target_ratio_a);
    _attack_base = (1.0F + _target_ratio_a) * (1.0F - _attack_coef);
}

inline auto adsr::set_decay(float rate) -> void
{
    _decay_rate = rate;
    _decay_coef = calc_coef(rate, _target_ratio_dr);
    _decay_base = (_sustain_level - _target_ratio_dr) * (1.0F - _decay_coef);
}

inline auto adsr::set_sustain(float level) -> void
{
    _sustain_level = level;
    _decay_base    = (_sustain_level - _target_ratio_dr) * (1.0F - _decay_coef);
}

inline auto adsr::set_release(float rate) -> void
{
    _release_rate = rate;
    _release_coef = calc_coef(rate, _target_ratio_dr);
    _release_base = -_target_ratio_dr * (1.0F - _release_coef);
}

inline auto adsr::set_target_ratio_a(float ratio) -> void
{
    if (ratio < 0.000000001) {
        ratio = 0.000000001;  // -180 dB
    }
    _target_ratio_a = ratio;
    _attack_base    = (1.0 + _target_ratio_a) * (1.0 - _attack_coef);
}

inline auto adsr::set_target_ratio_dr(float ratio) -> void
{
    if (ratio < 0.000000001) {
        ratio = 0.000000001;  // -180 dB
    }
    _target_ratio_dr = ratio;
    _decay_base      = (_sustain_level - _target_ratio_dr) * (1.0 - _decay_coef);
    _release_base    = -_target_ratio_dr * (1.0 - _release_coef);
}

inline auto adsr::gate(bool is_on) -> void
{
    if (is_on) {
        _state = state::Attack;
    } else if (_state != state::Idle) {
        _state = state::Release;
    }
}

inline auto adsr::reset() -> void
{
    _state  = state::Idle;
    _output = 0.0;
}

inline auto adsr::process_sample() -> float
{
    switch (_state) {
        case state::Idle: {
            break;
        }
        case state::Attack: {
            _output = _attack_base + _output * _attack_coef;
            if (_output >= 1.0) {
                _output = 1.0;
                _state  = state::Decay;
            }
            break;
        }
        case state::Decay: {
            _output = _decay_base + _output * _decay_coef;
            if (_output <= _sustain_level) {
                _output = _sustain_level;
                _state  = state::Sustain;
            }
            break;
        }
        case state::Sustain: {
            break;
        }
        case state::Release: {
            _output = _release_base + _output * _release_coef;
            if (_output <= 0.0) {
                _output = 0.0;
                _state  = state::Idle;
            }
            break;
        }
    }

    return _output;
}

}  // namespace grit
