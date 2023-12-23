#pragma once

#include <etl/chrono.hpp>

namespace grit {

template<typename T>
using microseconds = etl::chrono::duration<T, etl::micro>;

template<typename T>
using milliseconds = etl::chrono::duration<T, etl::milli>;

template<typename T>
using seconds = etl::chrono::duration<T>;

}  // namespace grit
