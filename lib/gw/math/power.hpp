#pragma once

namespace gw
{

template<typename T>
auto power(T base, T exponent) -> T
{
    T result = 1;
    for (T i = 0; i < exponent; i++) { result *= base; }
    return result;
}

template<auto Base>
auto power(decltype(Base) exponent) -> decltype(Base)
{
    return power(Base, exponent);
}

}  // namespace gw
