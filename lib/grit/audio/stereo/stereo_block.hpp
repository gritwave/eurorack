#pragma once

#include <etl/concepts.hpp>
#include <etl/mdspan.hpp>

namespace grit {

/// \ingroup grit-audio-stereo
template<etl::floating_point Float>
using StereoBlock = etl::mdspan<Float, etl::extents<etl::size_t, 2, etl::dynamic_extent>, etl::layout_left>;

}  // namespace grit
