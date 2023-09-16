#pragma once

#include <etl/linalg.hpp>
#include <etl/mdspan.hpp>

namespace etl::linalg {
using etl::linalg::detail::in_vector;
using etl::linalg::detail::inout_vector;
using etl::linalg::detail::out_vector;

using etl::linalg::detail::in_matrix;
using etl::linalg::detail::inout_matrix;
using etl::linalg::detail::out_matrix;

using etl::linalg::detail::in_object;
using etl::linalg::detail::inout_object;
using etl::linalg::detail::out_object;
}  // namespace etl::linalg