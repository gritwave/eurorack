#pragma once

#include <etl/cstdint.hpp>

namespace grit::fft {

/// \ingroup grit-fft
enum struct Direction : etl::int8_t
{
    Forward  = -1,
    Backward = 1,
};

}  // namespace grit::fft
