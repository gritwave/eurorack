#pragma once

namespace gw
{

template<typename T>
auto ipow(T base, T exponent) -> T
{
    T result = 1;
    for (T i = 0; i < exponent; i++) { result *= base; }
    return result;
}

template<auto Base>
auto ipow(decltype(Base) exponent) -> decltype(Base)
{
    return ipow(Base, exponent);
}

}  // namespace gw
