#include <grit/audio.hpp>
#include <grit/core/benchmark.hpp>
#include <grit/fft.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/bit.hpp>
#include <etl/chrono.hpp>
#include <etl/functional.hpp>
#include <etl/linalg.hpp>
#include <etl/numeric.hpp>
#include <etl/random.hpp>

#include <daisy_patch_sm.h>

template<typename RealOrComplex, unsigned N>
[[nodiscard]] auto makeNoise(auto& rng) -> etl::array<RealOrComplex, N>
{
    if constexpr (etl::floating_point<RealOrComplex>) {
        using Float = RealOrComplex;
        auto buf    = etl::array<Float, N>{};
        auto dist   = etl::uniform_real_distribution<Float>{Float(-1.0), Float(1.0)};
        auto gen    = [&rng, &dist] { return dist(rng); };
        etl::generate(buf.begin(), buf.end(), gen);
        return buf;
    } else {
        using Float = RealOrComplex::value_type;
        auto buf    = etl::array<RealOrComplex, N>{};
        auto dist   = etl::uniform_real_distribution<Float>{Float(-1.0), Float(1.0)};
        auto gen    = [&rng, &dist] { return RealOrComplex{dist(rng), dist(rng)}; };
        etl::generate(buf.begin(), buf.end(), gen);
        return buf;
    }
};

template<int N, typename Benchmark>
auto fftBench(char const* name, Benchmark bench)
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

template<int BlockSize, typename Benchmark>
auto audioBench(char const* name, Benchmark bench)
{
    static constexpr auto Runs = 128;

    using Microseconds = etl::chrono::duration<float, etl::micro>;

    auto runs = etl::array<float, Runs>{};

    auto rng              = etl::xoshiro128plusplus{14342};
    auto const noiseLeft  = makeNoise<float, BlockSize>(rng);
    auto const noiseRight = makeNoise<float, BlockSize>(rng);

    auto const fillWithNoise = [&](auto& block) {
        for (auto i{0U}; i < block.extent(1); ++i) {
            block(0, i) = noiseLeft[i];
            block(1, i) = noiseRight[i];
        }
    };

    auto buffer = etl::array<float, BlockSize * 2>{};
    auto block  = grit::StereoBlock<float>{buffer.data(), buffer.size() / 2};

    for (auto i{0U}; i < Runs; ++i) {
        fillWithNoise(block);

        auto const start = daisy::System::GetUs();
        bench(block);
        auto const stop = daisy::System::GetUs();

        grit::doNotOptimize(buffer.front());
        grit::doNotOptimize(buffer.back());

        runs[i] = etl::chrono::duration_cast<Microseconds>(etl::chrono::microseconds{stop - start}).count();
    }

    auto const average = int(etl::reduce(runs.begin(), end(runs), 0.0F) / static_cast<float>(runs.size()));

    daisy::patch_sm::DaisyPatchSM::PrintLine(
        "%30s Block: %d - Runs: %4d - Average: %4d us - Min: %4d us - Max: %4d us\n",
        name,
        BlockSize,
        Runs,
        average,
        int(*etl::min_element(runs.begin(), runs.end())),
        int(*etl::max_element(runs.begin(), runs.end()))
    );
}

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
    grit::fft::StaticComplexPlanV2<etl::complex<Float>, N> _plan{};
    etl::array<etl::complex<Float>, N> _buf{[] {
        auto rng = etl::xoshiro128plusplus{42};
        return makeNoise<etl::complex<Float>, N>(rng);
    }()};
};

template<typename Processor>
struct StereoProcessor
{
    explicit StereoProcessor(float sampleRate)
    {
        if constexpr (requires { _left.setSampleRate(sampleRate); }) {
            _left.setSampleRate(sampleRate);
            _right.setSampleRate(sampleRate);
        }
    }

    auto operator()(grit::StereoBlock<float> const& block) -> void
    {
        for (auto i{0U}; i < block.extent(1); ++i) {
            block(0, i) = _left(block(0, i));
            block(1, i) = _right(block(1, i));
        }
    }

private:
    Processor _left;
    Processor _right;
};

namespace mcu {

auto patch = daisy::patch_sm::DaisyPatchSM{};

}  // namespace mcu

auto main() -> int
{
    mcu::patch.Init();

    daisy::patch_sm::DaisyPatchSM::StartLog(true);
    daisy::patch_sm::DaisyPatchSM::PrintLine("Daisy Patch SM started. Test Beginning");

    audioBench<16>("AirWindowsFireAmp:     ", StereoProcessor<grit::AirWindowsFireAmp<float>>{96'000.0F});
    audioBench<16>("AirWindowsGrindAmp:    ", StereoProcessor<grit::AirWindowsGrindAmp<float>>{96'000.0F});
    audioBench<16>("AirWindowsVinylDither: ", StereoProcessor<grit::AirWindowsVinylDither<float>>{96'000.0F});
    daisy::patch_sm::DaisyPatchSM::PrintLine("");

    audioBench<32>("AirWindowsFireAmp:     ", StereoProcessor<grit::AirWindowsFireAmp<float>>{96'000.0F});
    audioBench<32>("AirWindowsGrindAmp:    ", StereoProcessor<grit::AirWindowsGrindAmp<float>>{96'000.0F});
    audioBench<32>("AirWindowsVinylDither: ", StereoProcessor<grit::AirWindowsVinylDither<float>>{96'000.0F});
    daisy::patch_sm::DaisyPatchSM::PrintLine("");

    audioBench<64>("AirWindowsFireAmp:     ", StereoProcessor<grit::AirWindowsFireAmp<float>>{96'000.0F});
    audioBench<64>("AirWindowsGrindAmp:    ", StereoProcessor<grit::AirWindowsGrindAmp<float>>{96'000.0F});
    audioBench<64>("AirWindowsVinylDither: ", StereoProcessor<grit::AirWindowsVinylDither<float>>{96'000.0F});
    daisy::patch_sm::DaisyPatchSM::PrintLine("");

    // fftBench<64>("ComplexRoundtrip<float, 16, v3>      - ", ComplexRoundtrip<float, 16, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 32, v3>      - ", ComplexRoundtrip<float, 32, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 64, v3>      - ", ComplexRoundtrip<float, 64, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 128, v3>     - ", ComplexRoundtrip<float, 128, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 256, v3>     - ", ComplexRoundtrip<float, 256, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 512, v3>     - ", ComplexRoundtrip<float, 512, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 1024, v3>    - ", ComplexRoundtrip<float, 1024, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 2048, v3>    - ", ComplexRoundtrip<float, 2048, c2c_dit2_v3>{});
    // fftBench<64>("ComplexRoundtrip<float, 4096, v3>    - ", ComplexRoundtrip<float, 4096, c2c_dit2_v3>{});
    // daisy::patch_sm::DaisyPatchSM::PrintLine("");

    // fftBench<64>("StaticComplexRoundtrip<float, 64>   - ", StaticComplexRoundtrip<float, 64>{});
    // fftBench<64>("StaticComplexRoundtrip<float, 128>  - ", StaticComplexRoundtrip<float, 128>{});
    // fftBench<64>("StaticComplexRoundtrip<float, 256>  - ", StaticComplexRoundtrip<float, 256>{});
    // fftBench<64>("StaticComplexRoundtrip<float, 512>  - ", StaticComplexRoundtrip<float, 512>{});
    // fftBench<64>("StaticComplexRoundtrip<float, 1024> - ", StaticComplexRoundtrip<float, 1024>{});
    // fftBench<64>("StaticComplexRoundtrip<float, 2048> - ", StaticComplexRoundtrip<float, 2048>{});
    // fftBench<64>("StaticComplexRoundtrip<float, 4096> - ", StaticComplexRoundtrip<float, 4096>{});
    // daisy::patch_sm::DaisyPatchSM::PrintLine("");

    while (true) {}
}
