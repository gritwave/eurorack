#pragma once

#include <etl/chrono.hpp>

namespace grit {

/// \ingroup grit-unit
template<typename T>
using Microseconds = etl::chrono::duration<T, etl::micro>;

/// \ingroup grit-unit
template<typename T>
using Milliseconds = etl::chrono::duration<T, etl::milli>;

/// \ingroup grit-unit
template<typename T>
using Seconds = etl::chrono::duration<T>;

}  // namespace grit
