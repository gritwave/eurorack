#pragma once

#include <grit/math/ilog2.hpp>

#include <etl/array.hpp>
#include <etl/bit.hpp>
#include <etl/concepts.hpp>
#include <etl/cstddef.hpp>
#include <etl/linalg.hpp>
#include <etl/type_traits.hpp>
#include <etl/utility.hpp>

namespace grit::fft {

/// \ingroup grit-fft
template<etl::size_t Size>
    requires(etl::has_single_bit(Size))
struct StaticBitrevorderPlan
{
    StaticBitrevorderPlan() = default;

    template<etl::linalg::inout_vector Vec>
    auto operator()(Vec x) -> void
    {
        for (auto i{0U}; i < _table.size(); ++i) {
            auto const j = static_cast<typename Vec::index_type>(_table[i]);
            if (i < j) {
                etl::swap(x(i), x(j));
            }
        }
    }

private:
    using index_type = etl::smallest_size_t<Size>;

    [[nodiscard]] static constexpr auto make() -> etl::array<index_type, Size>
    {
        auto const order = ilog2(Size);
        auto table       = etl::array<index_type, Size>{};
        for (auto i{0U}; i < Size; ++i) {
            for (auto j{0U}; j < order; ++j) {
                table[i] |= ((i >> j) & 1) << (order - 1 - j);
            }
        }
        return table;
    }

    etl::array<index_type, Size> _table{make()};
};

}  // namespace grit::fft
