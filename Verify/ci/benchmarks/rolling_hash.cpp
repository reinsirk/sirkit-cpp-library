// SPDX-License-Identifier: CC0-1.0
#include "benchmark_common.hpp"
#include "library.hpp"
#include <algorithm>
#include <vector>

namespace sirkit::detail::benchmark_rolling_hash {
constexpr std::uint64_t modulus = (std::uint64_t{1} << 61) - 1;
constexpr std::uint64_t base = 257;
std::uint64_t multiply(std::uint64_t a, std::uint64_t b) {
#if defined(__SIZEOF_INT128__)
    return static_cast<std::uint64_t>(static_cast<__uint128_t>(a) * b % modulus);
#else
    std::uint64_t result = 0;
    while (b) {
        if (b & 1) result = (result + a) % modulus;
        a = (a + a) % modulus;
        b >>= 1;
    }
    return result;
#endif
}
struct Input { std::string text; std::vector<std::pair<std::size_t, std::size_t>> ranges; };
Input generate(std::size_t n, std::size_t pattern) {
    std::uint64_t seed = 20261114;
    Input input;
    input.text.resize(n);
    for (auto& c : input.text)
        c = pattern ? 'a' : static_cast<char>('a' + ::sirkit::detail::benchmark_random(seed) % 26);
    input.ranges.reserve(n * 4);
    for (std::size_t i = 0; i < n * 4; ++i) {
        auto l = static_cast<std::size_t>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        auto r = static_cast<std::size_t>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        if (l > r) std::swap(l, r);
        input.ranges.emplace_back(l, r);
    }
    return input;
}
std::uint64_t reference(const Input& input) {
    std::vector<std::uint64_t> prefix(input.text.size() + 1), power(input.text.size() + 1, 1);
    for (std::size_t i = 0; i < input.text.size(); ++i) {
        prefix[i + 1] = (multiply(prefix[i], base) + static_cast<unsigned char>(input.text[i])) % modulus;
        power[i + 1] = multiply(power[i], base);
    }
    std::uint64_t checksum = 0;
    for (auto [l, r] : input.ranges) {
        const auto answer = (prefix[r] + modulus - multiply(prefix[l], power[r - l])) % modulus;
        checksum = ::sirkit::detail::benchmark_digest(checksum, answer);
    }
    return checksum;
}
std::uint64_t kernel(const Input& input) {
    RollingHash hash(input.text, base);
    std::uint64_t checksum = 0;
    for (auto [l, r] : input.ranges)
        checksum = ::sirkit::detail::benchmark_digest(checksum, hash.get(l, r));
    return checksum;
}
}
int main(int argc, char** argv) {
    return ::sirkit::detail::benchmark_main(argc, argv, ::sirkit::detail::benchmark_rolling_hash::generate,
        ::sirkit::detail::benchmark_rolling_hash::reference, ::sirkit::detail::benchmark_rolling_hash::kernel);
}
