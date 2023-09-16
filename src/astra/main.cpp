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

namespace astra {

auto mcu = daisy::patch_sm::DaisyPatchSM{};

}  // namespace astra

namespace etl {
template<int N, typename Func>
auto timeit(char const* name, Func func)
{
    using microseconds_t = etl::chrono::duration<float, etl::micro>;

    auto runs = etl::array<float, N>{};

    func();
    func();
    func();

    for (auto i{0U}; i < N; ++i) {
        auto const start = astra::mcu.system.GetUs();
        func();
        auto const stop = astra::mcu.system.GetUs();

        runs[i] = etl::chrono::duration_cast<microseconds_t>(etl::chrono::microseconds{stop - start}).count();
    }

    astra::mcu.PrintLine(
        "%30s Runs: %4d - Average: %4d us - Min: %4d us - Max: %4d us\n",
        name,
        N,
        int(etl::reduce(runs.begin(), end(runs), 0.0F) / static_cast<float>(runs.size())),
        int(*etl::min_element(runs.begin(), runs.end())),
        int(*etl::max_element(runs.begin(), runs.end()))
    );
}
}  // namespace etl

// template<typename T, unsigned N>
// static auto make_random_float_buffer(auto& rng) -> etl::array<T, N>
// {
//     auto buf = etl::array<T, N>{};
//     auto gen = [&rng] { return etl::uniform_real_distribution<T>{T(-0.5), T(0.5)}(rng); };
//     etl::generate(buf.begin(), buf.end(), gen);
//     return buf;
// };

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

auto main() -> int
{
    using namespace astra;

    astra::mcu.Init();

    astra::mcu.StartLog(true);
    astra::mcu.PrintLine("Daisy Patch SM started. Test Beginning");

    etl::timeit<128>("c2c_dit2_kernel<float, 16>   - ", fft_benchmark<float, 16>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 32>   - ", fft_benchmark<float, 32>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 64>   - ", fft_benchmark<float, 64>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 128>  - ", fft_benchmark<float, 128>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 256>  - ", fft_benchmark<float, 256>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 512>  - ", fft_benchmark<float, 512>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 1024> - ", fft_benchmark<float, 1024>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 2048> - ", fft_benchmark<float, 2048>{});
    etl::timeit<128>("c2c_dit2_kernel<float, 4096> - ", fft_benchmark<float, 4096>{});

    while (true) {}
}
