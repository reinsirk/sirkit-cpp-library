// SPDX-License-Identifier: CC0-1.0
// 入力生成と独立参照は計測外。本体の構築・操作・破棄と結果の集約を計測する。
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#ifndef SIRKIT_CI_BENCHMARK_COMMON_DEFINED
#define SIRKIT_CI_BENCHMARK_COMMON_DEFINED
namespace sirkit::detail {
inline std::uint64_t benchmark_random(std::uint64_t& state) {
    state ^= state << 13; state ^= state >> 7; state ^= state << 17;
    return state;
}
inline std::uint64_t benchmark_digest(std::uint64_t digest, std::uint64_t value) {
    return (digest ^ value) * std::uint64_t{1099511628211};
}
inline std::size_t benchmark_argument(const char* text, std::size_t maximum) {
    std::size_t value = 0;
    const std::string_view s(text);
    const auto result = std::from_chars(s.data(), s.data() + s.size(), value);
    if (result.ec != std::errc{} || result.ptr != s.data() + s.size() || value > maximum)
        throw std::invalid_argument("invalid benchmark argument");
    return value;
}
template<class Generate, class Reference, class Kernel>
int benchmark_main(int argc, char** argv, Generate generate, Reference reference, Kernel kernel) {
    try {
        if (argc != 4) throw std::invalid_argument("expected size pattern rounds");
        const auto size = benchmark_argument(argv[1], 2000000);
        const auto pattern = benchmark_argument(argv[2], 1);
        const auto rounds = benchmark_argument(argv[3], 4096);
        if (size < 2 || rounds == 0) throw std::invalid_argument("empty benchmark");
        const auto input = generate(size, pattern);
        const auto expected_once = reference(input);
        std::uint64_t expected = 0;
        for (std::size_t i = 0; i < rounds; ++i) expected = benchmark_digest(expected, expected_once);
        const auto run = [&] {
            std::uint64_t checksum = 0;
            for (std::size_t i = 0; i < rounds; ++i)
                checksum = benchmark_digest(checksum, kernel(input));
            return checksum;
        };
        if (run() != expected) throw std::runtime_error("independent reference mismatch before timing");
        const auto start = std::chrono::steady_clock::now();
        const auto checksum = run();
        const auto stop = std::chrono::steady_clock::now();
        if (checksum != expected) throw std::runtime_error("independent reference mismatch after timing");
        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
        std::cout << "{\"elapsed_ns\":" << ns << ",\"checksum\":\"" << checksum
                  << "\",\"expected\":\"" << expected << "\"}\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
}
#endif
