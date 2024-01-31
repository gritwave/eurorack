#pragma once

#include <etl/chrono.hpp>

namespace grit {

template<typename T>
using Microseconds = etl::chrono::duration<T, etl::micro>;

template<typename T>
using Milliseconds = etl::chrono::duration<T, etl::milli>;

template<typename T>
using Seconds = etl::chrono::duration<T>;

}  // namespace grit
