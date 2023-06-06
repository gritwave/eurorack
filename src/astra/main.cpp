#include <ta/fft/radix2.hpp>

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

    auto runs    = etl::array<float, N>{};
    auto results = etl::array<decltype(func()), N>{};

    func();
    func();
    func();

    for (auto i{0U}; i < N; ++i)
    {
        auto const start  = astra::mcu.system.GetUs();
        auto const result = func();
        auto const stop   = astra::mcu.system.GetUs();

        results[i] = result;
        runs[i]    = etl::chrono::duration_cast<microseconds_t>(etl::chrono::microseconds{stop - start}).count();
    }

    astra::mcu.PrintLine("%30s Runs: %d - Average: %d us - Min: %d us - Max: %d us\n", name, etl::size(results),
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

auto main() -> int
{
    using namespace astra;

    astra::mcu.Init();

    astra::mcu.StartLog(true);
    astra::mcu.PrintLine("Daisy Patch SM started. Test Beginning");

    // {
    //     auto const tw = ta::fft::make_twiddle_factors<float, 16>();
    //     auto buf      = etl::array<etl::complex<float>, 16>{};

    //     etl::timeit<256>("ta::fft::radix2_inplace<float, 16>(buf, tw)",
    //                      [&buf, &tw]
    //                      {
    //                          etl::generate(buf.begin(), buf.end(),
    //                                        [i = 0]() mutable { return static_cast<float>(i++); });
    //                          ta::fft::radix2_inplace<float, 16>(buf, tw);
    //                          return buf.back();
    //                      });
    // }

    // {
    //     auto const tw = ta::fft::make_twiddle_factors<float, 64>();
    //     auto buf      = etl::array<etl::complex<float>, 64>{};

    //     etl::timeit<256>("ta::fft::radix2_inplace<float, 64>(buf, tw)",
    //                      [&buf, &tw]
    //                      {
    //                          etl::generate(buf.begin(), buf.end(),
    //                                        [i = 0]() mutable { return static_cast<float>(i++); });
    //                          ta::fft::radix2_inplace<float, 64>(buf, tw);
    //                          return buf.back();
    //                      });
    // }

    // {
    //     auto const tw = ta::fft::make_twiddle_factors<float, 128>();
    //     auto buf      = etl::array<etl::complex<float>, 128>{};

    //     etl::timeit<256>("ta::fft::radix2_inplace<float, 128>(buf, tw)",
    //                      [&buf, &tw]
    //                      {
    //                          etl::generate(buf.begin(), buf.end(),
    //                                        [i = 0]() mutable { return static_cast<float>(i++); });
    //                          ta::fft::radix2_inplace<float, 128>(buf, tw);
    //                          return buf.back();
    //                      });
    // }

    // {
    //     auto const tw = ta::fft::make_twiddle_factors<float, 256>();
    //     auto buf      = etl::array<etl::complex<float>, 256>{};

    //     etl::timeit<256>("ta::fft::radix2_inplace<float, 256>(buf, tw)",
    //                      [&buf, &tw]
    //                      {
    //                          etl::generate(buf.begin(), buf.end(),
    //                                        [i = 0]() mutable { return static_cast<float>(i++); });
    //                          ta::fft::radix2_inplace<float, 256>(buf, tw);
    //                          return buf.back();
    //                      });
    // }

    // {
    //     auto const tw = ta::fft::make_twiddle_factors<float, 512>();
    //     auto buf      = etl::array<etl::complex<float>, 512>{};

    //     etl::timeit<256>("ta::fft::radix2_inplace<float, 512>(buf, tw)",
    //                      [&buf, &tw]
    //                      {
    //                          etl::generate(buf.begin(), buf.end(),
    //                                        [i = 0]() mutable { return static_cast<float>(i++); });
    //                          ta::fft::radix2_inplace<float, 512>(buf, tw);
    //                          return buf.back();
    //                      });
    // }

    // {
    //     auto const tw = ta::fft::make_twiddle_factors<float, 1024>();
    //     auto buf      = etl::array<etl::complex<float>, 1024>{};

    //     etl::timeit<256>("ta::fft::radix2_inplace<float, 1024>(buf, tw)",
    //                      [&buf, &tw]
    //                      {
    //                          etl::generate(buf.begin(), buf.end(),
    //                                        [i = 0]() mutable { return static_cast<float>(i++); });
    //                          ta::fft::radix2_inplace<float, 1024>(buf, tw);
    //                          return buf.back();
    //                      });
    // }
    // {
    //     auto const tw = ta::fft::make_twiddle_factors<float, 2048>();
    //     auto buf      = etl::array<etl::complex<float>, 2048>{};

    //     etl::timeit<256>("ta::fft::radix2_inplace<float, 2048>(buf, tw)",
    //                      [&buf, &tw]
    //                      {
    //                          etl::generate(buf.begin(), buf.end(),
    //                                        [i = 0]() mutable { return static_cast<float>(i++); });
    //                          ta::fft::radix2_inplace<float, 2048>(buf, tw);
    //                          return buf.back();
    //                      });
    // }

    auto const seed = astra::mcu.GetRandomValue();
    astra::mcu.PrintLine("Random Seed: %d", static_cast<int>(seed));
    auto rng = etl::xoshiro128plusplus{seed};

    {
        auto lhs             = make_random_int_buffer<int32_t, 1024>(rng);
        auto rhs             = make_random_int_buffer<int32_t, 1024>(rng);
        auto const benchmark = [&lhs, &rhs]
        {
            etl::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), etl::divides{});
            return lhs.front() + lhs.back();
        };
        etl::timeit<256>("etl::array<int32_t, 1024> / etl::array<int32_t, 1024>", benchmark);
    }

    {
        auto lhs             = make_random_int_buffer<int32_t, 2048>(rng);
        auto rhs             = make_random_int_buffer<int32_t, 2048>(rng);
        auto const benchmark = [&lhs, &rhs]
        {
            etl::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), etl::divides{});
            return lhs.front() + lhs.back();
        };
        etl::timeit<256>("etl::array<int32_t, 2048> / etl::array<int32_t, 2048>", benchmark);
    }

    {
        auto lhs             = make_random_int_buffer<int32_t, 4096>(rng);
        auto rhs             = make_random_int_buffer<int32_t, 4096>(rng);
        auto const benchmark = [&lhs, &rhs]
        {
            etl::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), etl::divides{});
            return lhs.front() + lhs.back();
        };
        etl::timeit<256>("etl::array<int32_t, 4096> / etl::array<int32_t, 4096>", benchmark);
    }

    {
        auto lhs             = make_random_float_buffer<float, 1024>(rng);
        auto rhs             = make_random_float_buffer<float, 1024>(rng);
        auto const benchmark = [&lhs, &rhs]
        {
            etl::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), etl::divides{});
            return lhs.front() + lhs.back();
        };
        etl::timeit<256>("etl::array<float, 1024> / etl::array<float, 1024>", benchmark);
    }

    {
        auto lhs             = make_random_float_buffer<float, 2048>(rng);
        auto rhs             = make_random_float_buffer<float, 2048>(rng);
        auto const benchmark = [&lhs, &rhs]
        {
            etl::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), etl::divides{});
            return lhs.front() + lhs.back();
        };
        etl::timeit<256>("etl::array<float, 2048> / etl::array<float, 2048>", benchmark);
    }
    {
        auto lhs             = make_random_float_buffer<float, 4096>(rng);
        auto rhs             = make_random_float_buffer<float, 4096>(rng);
        auto const benchmark = [&lhs, &rhs]
        {
            etl::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), etl::divides{});
            return lhs.front() + lhs.back();
        };
        etl::timeit<256>("etl::array<float, 4096> / etl::array<float, 4096>", benchmark);
    }

    astra::mcu.SetAudioSampleRate(SAMPLE_RATE);
    astra::mcu.SetAudioBlockSize(BLOCK_SIZE);
    astra::mcu.StartAudio(audioCallback);

    while (true) {}
}
