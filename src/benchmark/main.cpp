#include <grit/audio/noise/dither.hpp>
#include <grit/core/benchmark.hpp>
#include <grit/fft/fft.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/bit.hpp>
#include <etl/chrono.hpp>
#include <etl/functional.hpp>
#include <etl/linalg.hpp>
#include <etl/numeric.hpp>
#include <etl/random.hpp>

#include <daisy_patch_sm.h>

namespace astra {

auto mcu = daisy::patch_sm::DaisyPatchSM{};

}  // namespace astra

namespace etl {
template<int N, typename Benchmark>
auto timeit(char const* name, Benchmark bench)
{
    using Microseconds = etl::chrono::duration<float, etl::micro>;

    auto runs = etl::array<float, N>{};

    bench();
    bench();
    bench();

    for (auto i{0U}; i < N; ++i) {
        auto const start = daisy::System::GetUs();
        bench();
        auto const stop = daisy::System::GetUs();

        runs[i] = etl::chrono::duration_cast<Microseconds>(etl::chrono::microseconds{stop - start}).count();
    }

    auto const average = int(etl::reduce(runs.begin(), end(runs), 0.0F) / static_cast<float>(runs.size()));
    auto const dsize   = double(bench.size());
    auto const mflops  = static_cast<int>(std::lround(5.0 * dsize * std::log2(dsize) / average)) * 2;

    daisy::patch_sm::DaisyPatchSM::PrintLine(
        "%30s Runs: %4d - Average: %4d us - Min: %4d us - Max: %4d us - MFLOPS: %4d us\n",
        name,
        N,
        average,
        int(*etl::min_element(runs.begin(), runs.end())),
        int(*etl::max_element(runs.begin(), runs.end())),
        mflops
    );
}
}  // namespace etl

template<typename Complex, unsigned N>
[[nodiscard]] auto makeNoise(auto& rng) -> etl::array<Complex, N>
{
    using Float = Complex::value_type;
    auto buf    = etl::array<Complex, N>{};
    auto dist   = etl::uniform_real_distribution<Float>{Float(-0.5), Float(0.5)};
    auto gen    = [&rng, &dist] { return Complex{dist(rng), dist(rng)}; };
    etl::generate(buf.begin(), buf.end(), gen);
    return buf;
};

struct c2c_dit2_v3
{
    c2c_dit2_v3() = default;

    template<etl::linalg::inout_vector Vec>
    auto operator()(Vec x, auto const& twiddles) const noexcept -> void
    {
        auto const size  = x.size();
        auto const order = grit::ilog2(size);

        {
            // stage 0
            static constexpr auto const stage_length = 1;  // grit::ipow<2>(0)
            static constexpr auto const stride       = 2;  // grit::ipow<2>(0 + 1)

            for (auto k{0}; k < static_cast<int>(size); k += stride) {
                auto const i1 = k;
                auto const i2 = k + stage_length;

                auto const temp = x(i1) + x(i2);
                x(i2)           = x(i1) - x(i2);
                x(i1)           = temp;
            }
        }

        for (auto stage{1ULL}; stage < order; ++stage) {

            auto const stage_length = grit::ipow<2ULL>(stage);
            auto const stride       = grit::ipow<2ULL>(stage + 1);
            auto const tw_stride    = grit::ipow<2ULL>(order - stage - 1ULL);

            for (auto k{0ULL}; k < size; k += stride) {
                for (auto pair{0ULL}; pair < stage_length; ++pair) {
                    auto const tw = twiddles(pair * tw_stride);

                    auto const i1 = k + pair;
                    auto const i2 = k + pair + stage_length;

                    auto const temp = x(i1) + tw * x(i2);
                    x(i2)           = x(i1) - tw * x(i2);
                    x(i1)           = temp;
                }
            }
        }
    }
};

template<typename Float, int N, typename Kernel>
struct ComplexRoundtrip
{
    ComplexRoundtrip() = default;

    static constexpr auto size() { return N; }

    auto operator()() -> void
    {
        auto x      = etl::mdspan<etl::complex<Float>, etl::extents<etl::size_t, N>>{_buf.data()};
        auto w      = etl::mdspan<etl::complex<Float> const, etl::extents<etl::size_t, N / 2>>{_tw.data()};
        auto kernel = Kernel{};

        kernel(x, w);
        kernel(x, etl::linalg::conjugated(w));
        etl::linalg::scale(Float(1) / Float(N), x);

        grit::doNotOptimize(_buf.front());
        grit::doNotOptimize(_buf.back());
    }

private:
    etl::array<etl::complex<Float>, N / 2> _tw{grit::fft::detail::makeTwiddles<Float, N>()};
    etl::array<etl::complex<Float>, N> _buf{[] {
        auto rng = etl::xoshiro128plusplus{42};
        return makeNoise<etl::complex<Float>, N>(rng);
    }()};
};

template<typename Float, int N>
struct StaticComplexRoundtrip
{
    StaticComplexRoundtrip() = default;

    static constexpr auto size() { return N; }

    auto operator()() -> void
    {
        auto x = etl::mdspan{_buf.data(), etl::extents<etl::size_t, N>{}};
        _plan(x, grit::fft::Direction::Forward);
        _plan(x, grit::fft::Direction::Backward);
        etl::linalg::scale(Float(1) / Float(N), x);

        grit::doNotOptimize(_buf.front());
        grit::doNotOptimize(_buf.back());
    }

private:
    grit::fft::ComplexPlanV2<etl::complex<Float>, N> _plan{};
    etl::array<etl::complex<Float>, N> _buf{[] {
        auto rng = etl::xoshiro128plusplus{42};
        return makeNoise<etl::complex<Float>, N>(rng);
    }()};
};

auto main() -> int
{
    using namespace astra;

    astra::mcu.Init();

    daisy::patch_sm::DaisyPatchSM::StartLog(true);
    daisy::patch_sm::DaisyPatchSM::PrintLine("Daisy Patch SM started. Test Beginning");

    etl::timeit<64>("ComplexRoundtrip<float, 16, v3>      - ", ComplexRoundtrip<float, 16, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 32, v3>      - ", ComplexRoundtrip<float, 32, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 64, v3>      - ", ComplexRoundtrip<float, 64, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 128, v3>     - ", ComplexRoundtrip<float, 128, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 256, v3>     - ", ComplexRoundtrip<float, 256, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 512, v3>     - ", ComplexRoundtrip<float, 512, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 1024, v3>    - ", ComplexRoundtrip<float, 1024, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 2048, v3>    - ", ComplexRoundtrip<float, 2048, c2c_dit2_v3>{});
    etl::timeit<64>("ComplexRoundtrip<float, 4096, v3>    - ", ComplexRoundtrip<float, 4096, c2c_dit2_v3>{});
    daisy::patch_sm::DaisyPatchSM::PrintLine("");

    // etl::timeit<64>("StaticComplexRoundtrip<float, 64>   - ", StaticComplexRoundtrip<float, 64>{});
    // etl::timeit<64>("StaticComplexRoundtrip<float, 128>  - ", StaticComplexRoundtrip<float, 128>{});
    // etl::timeit<64>("StaticComplexRoundtrip<float, 256>  - ", StaticComplexRoundtrip<float, 256>{});
    // etl::timeit<64>("StaticComplexRoundtrip<float, 512>  - ", StaticComplexRoundtrip<float, 512>{});
    // etl::timeit<64>("StaticComplexRoundtrip<float, 1024> - ", StaticComplexRoundtrip<float, 1024>{});
    // etl::timeit<64>("StaticComplexRoundtrip<float, 2048> - ", StaticComplexRoundtrip<float, 2048>{});
    // etl::timeit<64>("StaticComplexRoundtrip<float, 4096> - ", StaticComplexRoundtrip<float, 4096>{});
    // daisy::patch_sm::DaisyPatchSM::PrintLine("");

    while (true) {}
}
