#include <gw/audio/dither/no_dither.hpp>
#include <gw/audio/dither/rectangle_dither.hpp>
#include <gw/audio/dither/triangle_dither.hpp>
#include <gw/core/benchmark.hpp>
#include <gw/fft/fft.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/bit.hpp>
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
        gw::fft::c2c_dit2_kernel<Float, N>(_buf, _tw);
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
        gw::fft::c2c_dit2_kernel<N>(_buf, _tw);
        gw::doNotOptimize(_buf.front());
        gw::doNotOptimize(_buf.back());
    }

private:
    etl::array<gw::complex_q15_t, N> _buf{};
    etl::array<gw::complex_q15_t, N / 2> _tw{};
};

template<etl::size_t N>
struct cmul
{
    cmul()
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
struct cmul_sdram
{
    explicit cmul_sdram(etl::span<etl::complex<float>> sdram_buffer)
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

struct complex8_t
{
    static constexpr auto const scale     = 127.0F;
    static constexpr auto const inv_scale = 1.0F / scale;

    constexpr complex8_t() = default;

    constexpr complex8_t(float re, float im) noexcept
        : real{static_cast<etl::int8_t>(re * scale)}, imag{static_cast<etl::int8_t>(im * scale)}
    {
    }

    [[nodiscard]] constexpr operator etl::complex<float>() const noexcept
    {
        return etl::complex<float>{
            static_cast<float>(real) * inv_scale,
            static_cast<float>(imag) * inv_scale,
        };
    }

    [[nodiscard]] constexpr auto to_float() const noexcept { return static_cast<etl::complex<float>>(*this); }

    etl::int8_t real;
    etl::int8_t imag;
};

struct complex16_t
{
    static constexpr auto const scale     = 32767.0F;
    static constexpr auto const inv_scale = 1.0F / scale;

    constexpr complex16_t() = default;

    constexpr complex16_t(float re, float im) noexcept
        : real{static_cast<etl::int16_t>(re * scale)}, imag{static_cast<etl::int16_t>(im * scale)}
    {
    }

    [[nodiscard]] constexpr operator etl::complex<float>() const noexcept
    {
        return etl::complex<float>{
            static_cast<float>(real) * inv_scale,
            static_cast<float>(imag) * inv_scale,
        };
    }

    [[nodiscard]] constexpr auto to_float() const noexcept { return static_cast<etl::complex<float>>(*this); }

    etl::int16_t real;
    etl::int16_t imag;
};

template<typename CompressedComplex, etl::size_t N>
struct cmul_compressed
{
    cmul_compressed(etl::span<CompressedComplex> ints, etl::span<etl::complex<float>> floats)
        : _lhs{ints.data(), N}, _rhs{ints.data() + N, N}, _out{floats.data(), N}
    {
        auto rng  = etl::xoshiro128plusplus{astra::mcu.GetRandomValue()};
        auto dist = etl::uniform_real_distribution<float>{-1.0F, 1.0F};
        auto gen  = [&rng, &dist]() { return CompressedComplex{dist(rng), dist(rng)}; };
        etl::generate(_lhs.begin(), _lhs.end(), gen);
        etl::generate(_rhs.begin(), _rhs.end(), gen);
    }

    auto operator()() -> void
    {
        auto const mul = [](auto lhs, auto rhs) { return lhs.to_float() * rhs.to_float(); };
        etl::transform(_lhs.begin(), _lhs.end(), _rhs.begin(), _out.begin(), mul);
        gw::doNotOptimize(_out.front());
        gw::doNotOptimize(_out.back());
    }

private:
    etl::span<CompressedComplex, N> _lhs{};
    etl::span<CompressedComplex, N> _rhs{};
    etl::span<etl::complex<float>, N> _out{};
};

etl::complex<float> __attribute__((section(".sdram_bss"))) sdram_cplx_f32[32768 * 8];
complex16_t __attribute__((section(".sdram_bss"))) sdram_cplx_s16[32768 * 8];
complex8_t __attribute__((section(".sdram_bss"))) sdram_cplx_s8[32768 * 8];

auto main() -> int
{
    using namespace astra;

    astra::mcu.Init();

    astra::mcu.StartLog(true);
    astra::mcu.PrintLine("Daisy Patch SM started. Test Beginning");

    // etl::timeit<128>("c2c_dit2_kernel<float, 16>   - ", fft_benchmark<float, 16>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 32>   - ", fft_benchmark<float, 32>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 64>   - ", fft_benchmark<float, 64>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 128>  - ", fft_benchmark<float, 128>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 256>  - ", fft_benchmark<float, 256>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 512>  - ", fft_benchmark<float, 512>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 1024> - ", fft_benchmark<float, 1024>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 2048> - ", fft_benchmark<float, 2048>{});
    // etl::timeit<128>("c2c_dit2_kernel<float, 4096> - ", fft_benchmark<float, 4096>{});

    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 16>   - ", fft_benchmark_q15<16>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 32>   - ", fft_benchmark_q15<32>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 64>   - ", fft_benchmark_q15<64>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 128>  - ", fft_benchmark_q15<128>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 256>  - ", fft_benchmark_q15<256>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 512>  - ", fft_benchmark_q15<512>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 1024> - ", fft_benchmark_q15<1024>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 2048> - ", fft_benchmark_q15<2048>{});
    // etl::timeit<128>("c2c_dit2_kernel<q15_t, 4096> - ", fft_benchmark_q15<4096>{});

    // etl::timeit<256>("cmul<64>    - ", cmul<64>{});
    // etl::timeit<256>("cmul<128>   - ", cmul<128>{});
    // etl::timeit<256>("cmul<256>   - ", cmul<256>{});
    // etl::timeit<256>("cmul<512>   - ", cmul<512>{});
    // etl::timeit<256>("cmul<1024>  - ", cmul<1024>{});
    // etl::timeit<256>("cmul<2048>  - ", cmul<2048>{});
    // etl::timeit<256>("cmul<4096>  - ", cmul<4096>{});
    // etl::timeit<128>("cmul<8192>  - ", cmul<8192>{});
    // etl::timeit<128>("cmul<16384> - ", cmul<16384>{});
    // etl::timeit<128>("cmul<32768> - ", cmul<32768>{});
    etl::timeit<128>("cmul_complex8_t<32768>  - ", cmul_compressed<complex8_t, 32768>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<16384>  - ", cmul_compressed<complex8_t, 16384>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<8192>   - ", cmul_compressed<complex8_t, 8192>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<4096>   - ", cmul_compressed<complex8_t, 4096>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<2048>   - ", cmul_compressed<complex8_t, 2048>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<1024>   - ", cmul_compressed<complex8_t, 1024>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<512>    - ", cmul_compressed<complex8_t, 512>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<256>    - ", cmul_compressed<complex8_t, 256>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<128>    - ", cmul_compressed<complex8_t, 128>{sdram_cplx_s8, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex8_t<64>     - ", cmul_compressed<complex8_t, 64>{sdram_cplx_s8, sdram_cplx_f32});

    etl::timeit<128>("cmul_complex16_t<32768> - ", cmul_compressed<complex16_t, 32768>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<16384> - ", cmul_compressed<complex16_t, 16384>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<8192>  - ", cmul_compressed<complex16_t, 8192>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<4096>  - ", cmul_compressed<complex16_t, 4096>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<2048>  - ", cmul_compressed<complex16_t, 2048>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<1024>  - ", cmul_compressed<complex16_t, 1024>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<512>   - ", cmul_compressed<complex16_t, 512>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<256>   - ", cmul_compressed<complex16_t, 256>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<128>   - ", cmul_compressed<complex16_t, 128>{sdram_cplx_s16, sdram_cplx_f32});
    etl::timeit<128>("cmul_complex16_t<64>    - ", cmul_compressed<complex16_t, 64>{sdram_cplx_s16, sdram_cplx_f32});

    etl::timeit<128>("cmul_float_sdram<32768> - ", cmul_sdram<32768>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<16384> - ", cmul_sdram<16384>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<8192>  - ", cmul_sdram<8192>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<4096>  - ", cmul_sdram<4096>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<2048>  - ", cmul_sdram<2048>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<1024>  - ", cmul_sdram<1024>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<512>   - ", cmul_sdram<512>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<256>   - ", cmul_sdram<256>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<128>   - ", cmul_sdram<128>{sdram_cplx_f32});
    etl::timeit<128>("cmul_float_sdram<64>    - ", cmul_sdram<64>{sdram_cplx_f32});

    while (true) {}
}
