#include <gw/core/benchmark.hpp>
#include <gw/fft/radix2.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/chrono.hpp>
#include <etl/functional.hpp>
#include <etl/numeric.hpp>
#include <etl/random.hpp>

#include <daisy_patch_sm.h>

namespace astra
{

static constexpr auto BLOCK_SIZE  = 512U;
static constexpr auto SAMPLE_RATE = 44'100.0F;

auto mcu = daisy::patch_sm::DaisyPatchSM{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    mcu.ProcessAllControls();

    for (size_t i = 0; i < size; ++i)
    {
        auto const left  = IN_L[i];
        auto const right = IN_R[i];

        OUT_L[i] = left;
        OUT_R[i] = right;
    }
}

}  // namespace astra

namespace etl
{
template<int N, typename Func>
auto timeit(char const* name, Func func)
{
    using microseconds_t = etl::chrono::duration<float, etl::micro>;

    auto runs = etl::array<float, N>{};

    func();
    func();
    func();

    for (auto i{0U}; i < N; ++i)
    {
        auto const start = astra::mcu.system.GetUs();
        func();
        auto const stop = astra::mcu.system.GetUs();

        runs[i] = etl::chrono::duration_cast<microseconds_t>(etl::chrono::microseconds{stop - start}).count();
    }

    astra::mcu.PrintLine("%30s Runs: %4d - Average: %4d us - Min: %4d us - Max: %4d us\n", name, N,
                         int(etl::reduce(runs.begin(), end(runs), 0.0F) / static_cast<float>(runs.size())),
                         int(*etl::min_element(runs.begin(), runs.end())),
                         int(*etl::max_element(runs.begin(), runs.end())));
}
}  // namespace etl

template<typename T, unsigned N>
static auto make_random_float_buffer(auto& rng) -> etl::array<T, N>
{
    auto buf = etl::array<T, N>{};
    auto gen = [&rng] { return etl::uniform_real_distribution<T>{T(-0.5), T(0.5)}(rng); };
    etl::generate(buf.begin(), buf.end(), gen);
    return buf;
};

template<typename T, unsigned N>
static auto make_random_int_buffer(auto& rng) -> etl::array<T, N>
{
    auto buf = etl::array<T, N>{};
    auto gen = [&rng]
    {
        auto x = etl::uniform_int_distribution<T>{-9999, 9999}(rng);
        if (x == 0) { return T(1); }  // for divide benchmarks
        return x;
    };
    etl::generate(buf.begin(), buf.end(), gen);
    return buf;
};

template<typename Float, int N>
struct fft_benchmark
{
    fft_benchmark() = default;

    auto operator()() -> void
    {
        etl::generate(_buf.begin(), _buf.end(), [i = 0]() mutable { return static_cast<Float>(i++); });
        gw::fft::radix2_inplace<Float, N>(_buf, _tw);
        gw::doNotOptimize(_buf.front());
        gw::doNotOptimize(_buf.back());
    }

private:
    etl::array<etl::complex<Float>, N> _buf{};
    etl::array<etl::complex<Float>, N / 2> _tw{gw::fft::make_twiddle_factors<float, N>()};
};

template<int N>
struct fft_benchmark_fixed
{
    fft_benchmark_fixed()
    {
        auto rng  = etl::xoshiro128plusplus{astra::mcu.GetRandomValue()};
        auto dist = etl::uniform_int_distribution<int16_t>{-9999, 9999};
        auto gen  = [&rng, &dist]()
        {
            auto const real = dist(rng);
            auto const imag = dist(rng);
            return gw::complex_q15_t{
                real == 0 ? int16_t(1) : real,
                imag == 0 ? int16_t(1) : imag,
            };
        };

        etl::generate(_tw.begin(), _tw.end(), gen);
    }

    auto operator()() -> void
    {
        etl::generate(_buf.begin(), _buf.end(), [i = 0]() mutable { return static_cast<int16_t>(i++); });
        gw::fft::radix2_inplace<N>(_buf, _tw);
        gw::doNotOptimize(_buf.front());
        gw::doNotOptimize(_buf.back());
    }

private:
    etl::array<gw::complex_q15_t, N> _buf{};
    etl::array<gw::complex_q15_t, N / 2> _tw{};
};

auto main() -> int
{
    using namespace astra;

    astra::mcu.Init();

    astra::mcu.StartLog(true);
    astra::mcu.PrintLine("Daisy Patch SM started. Test Beginning");

    etl::timeit<1024>("radix2_inplace<float, 16>   - ", fft_benchmark<float, 16>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 16>   - ", fft_benchmark_fixed<16>{});
    etl::timeit<1024>("radix2_inplace<float, 32>   - ", fft_benchmark<float, 32>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 32>   - ", fft_benchmark_fixed<32>{});
    etl::timeit<1024>("radix2_inplace<float, 64>   - ", fft_benchmark<float, 64>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 64>   - ", fft_benchmark_fixed<64>{});
    etl::timeit<1024>("radix2_inplace<float, 128>  - ", fft_benchmark<float, 128>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 128>  - ", fft_benchmark_fixed<128>{});
    etl::timeit<1024>("radix2_inplace<float, 256>  - ", fft_benchmark<float, 256>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 256>  - ", fft_benchmark_fixed<256>{});
    etl::timeit<1024>("radix2_inplace<float, 512>  - ", fft_benchmark<float, 512>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 512>  - ", fft_benchmark_fixed<512>{});
    etl::timeit<1024>("radix2_inplace<float, 1024> - ", fft_benchmark<float, 1024>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 1024> - ", fft_benchmark_fixed<1024>{});
    etl::timeit<1024>("radix2_inplace<float, 2048> - ", fft_benchmark<float, 2048>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 2048> - ", fft_benchmark_fixed<2048>{});
    etl::timeit<1024>("radix2_inplace<float, 4096> - ", fft_benchmark<float, 4096>{});
    etl::timeit<1024>("radix2_inplace<q15_t, 4096> - ", fft_benchmark_fixed<4096>{});

    astra::mcu.SetAudioSampleRate(SAMPLE_RATE);
    astra::mcu.SetAudioBlockSize(BLOCK_SIZE);
    astra::mcu.StartAudio(audioCallback);

    while (true) {}
}
