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

auto mcu = daisy::patch_sm::DaisyPatchSM{};

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
struct fft_benchmark_q15
{
    fft_benchmark_q15()
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

template<etl::size_t N>
struct complex_multiply
{
    complex_multiply()
    {
        auto rng  = etl::xoshiro128plusplus{astra::mcu.GetRandomValue()};
        auto dist = etl::uniform_real_distribution<float>{-1.0F, 1.0F};
        auto gen  = [&rng, &dist]() { return etl::complex<float>{dist(rng), dist(rng)}; };
        etl::generate(_lhs.begin(), _lhs.end(), gen);
        etl::generate(_rhs.begin(), _rhs.end(), gen);
    }

    auto operator()() -> void
    {
        etl::transform(_lhs.begin(), _lhs.end(), _rhs.begin(), _out.begin(), etl::multiplies{});
        gw::doNotOptimize(_out.front());
        gw::doNotOptimize(_out.back());
    }

private:
    etl::array<etl::complex<float>, N> _lhs{};
    etl::array<etl::complex<float>, N> _rhs{};
    etl::array<etl::complex<float>, N> _out{};
};

template<etl::size_t N>
struct complex_multiply_sdram
{
    explicit complex_multiply_sdram(etl::span<etl::complex<float>> sdram_buffer)
        : _lhs{sdram_buffer.data(), N}, _rhs{sdram_buffer.data() + N, N}, _out{sdram_buffer.data() + N + N, N}
    {
        auto rng  = etl::xoshiro128plusplus{astra::mcu.GetRandomValue()};
        auto dist = etl::uniform_real_distribution<float>{-1.0F, 1.0F};
        auto gen  = [&rng, &dist]() { return etl::complex<float>{dist(rng), dist(rng)}; };
        etl::generate(_lhs.begin(), _lhs.end(), gen);
        etl::generate(_rhs.begin(), _rhs.end(), gen);
    }

    auto operator()() -> void
    {
        etl::transform(_lhs.begin(), _lhs.end(), _rhs.begin(), _out.begin(), etl::multiplies{});
        gw::doNotOptimize(_out.front());
        gw::doNotOptimize(_out.back());
    }

private:
    etl::span<etl::complex<float>, N> _lhs{};
    etl::span<etl::complex<float>, N> _rhs{};
    etl::span<etl::complex<float>, N> _out{};
};

etl::complex<float> __attribute__((section(".sdram_bss"))) sdram_buffer[32768 * 8];

auto main() -> int
{
    using namespace astra;

    astra::mcu.Init();

    astra::mcu.StartLog(true);
    astra::mcu.PrintLine("Daisy Patch SM started. Test Beginning");

    etl::timeit<128>("radix2_inplace<float, 16>   - ", fft_benchmark<float, 16>{});
    etl::timeit<128>("radix2_inplace<float, 32>   - ", fft_benchmark<float, 32>{});
    etl::timeit<128>("radix2_inplace<float, 64>   - ", fft_benchmark<float, 64>{});
    etl::timeit<128>("radix2_inplace<float, 128>  - ", fft_benchmark<float, 128>{});
    etl::timeit<128>("radix2_inplace<float, 256>  - ", fft_benchmark<float, 256>{});
    etl::timeit<128>("radix2_inplace<float, 512>  - ", fft_benchmark<float, 512>{});
    etl::timeit<128>("radix2_inplace<float, 1024> - ", fft_benchmark<float, 1024>{});
    etl::timeit<128>("radix2_inplace<float, 2048> - ", fft_benchmark<float, 2048>{});
    // etl::timeit<128>("radix2_inplace<float, 4096> - ", fft_benchmark<float, 4096>{});

    // etl::timeit<128>("radix2_inplace<q15_t, 16>   - ", fft_benchmark_q15<16>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 32>   - ", fft_benchmark_q15<32>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 64>   - ", fft_benchmark_q15<64>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 128>  - ", fft_benchmark_q15<128>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 256>  - ", fft_benchmark_q15<256>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 512>  - ", fft_benchmark_q15<512>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 1024> - ", fft_benchmark_q15<1024>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 2048> - ", fft_benchmark_q15<2048>{});
    // etl::timeit<128>("radix2_inplace<q15_t, 4096> - ", fft_benchmark_q15<4096>{});

    // etl::timeit<256>("complex_multiply<64>    - ", complex_multiply<64>{});
    // etl::timeit<256>("complex_multiply<128>   - ", complex_multiply<128>{});
    // etl::timeit<256>("complex_multiply<256>   - ", complex_multiply<256>{});
    // etl::timeit<256>("complex_multiply<512>   - ", complex_multiply<512>{});
    // etl::timeit<256>("complex_multiply<1024>  - ", complex_multiply<1024>{});
    // etl::timeit<256>("complex_multiply<2048>  - ", complex_multiply<2048>{});
    // etl::timeit<256>("complex_multiply<4096>  - ", complex_multiply<4096>{});
    // etl::timeit<128>("complex_multiply<8192>  - ", complex_multiply<8192>{});
    // etl::timeit<128>("complex_multiply<16384> - ", complex_multiply<16384>{});
    // etl::timeit<128>("complex_multiply<32768> - ", complex_multiply<32768>{});

    // etl::timeit<256>("complex_multiply_sdram<65536> - ", complex_multiply_sdram<65536>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<32768> - ", complex_multiply_sdram<32768>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<16384> - ", complex_multiply_sdram<16384>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<8192>  - ", complex_multiply_sdram<8192>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<4096>  - ", complex_multiply_sdram<4096>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<2048>  - ", complex_multiply_sdram<2048>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<1024>  - ", complex_multiply_sdram<1024>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<512>   - ", complex_multiply_sdram<512>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<256>   - ", complex_multiply_sdram<256>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<128>   - ", complex_multiply_sdram<128>{sdram_buffer});
    // etl::timeit<256>("complex_multiply_sdram<64>    - ", complex_multiply_sdram<64>{sdram_buffer});

    while (true) {}
}
