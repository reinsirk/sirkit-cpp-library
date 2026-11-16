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
struct Input {
    std::size_t pattern = 0;
    std::string text, other;
    std::vector<std::pair<std::size_t, std::size_t>> ranges;
};
Input generate(std::size_t n, std::size_t pattern) {
    std::uint64_t seed = 20261114;
    Input input;
    input.pattern = pattern;
    input.text.resize(n);
    for (auto& c : input.text)
        c = pattern ? 'a' : static_cast<char>('a' + ::sirkit::detail::benchmark_random(seed) % 26);
    if (pattern == 2) {
        input.other = input.text;
        input.other.back() = 'b';
        // 長い一致の最後だけ違う。直接文字走査による参照は使わず閉形式で答える。
        const auto queries = std::max<std::size_t>(32, n / 32);
        input.ranges.reserve(queries);
        for (std::size_t i = 0; i < queries; ++i) {
            const auto l = i % 8 == 0 ? n : i % 8 == 1 ? 0
                : static_cast<std::size_t>(::sirkit::detail::benchmark_random(seed) % n);
            input.ranges.emplace_back(l, n - l);
        }
        return input;
    }
    if (pattern == 3)
        for (std::size_t i = 0; i < n; ++i) input.text[i] = static_cast<char>((i % 4) * 85);
    input.ranges.reserve(n * 4);
    for (std::size_t i = 0; i < n * 4; ++i) {
        auto l = static_cast<std::size_t>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        auto r = static_cast<std::size_t>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        if (l > r) std::swap(l, r);
        if (pattern == 3) {
            if (i % 8 == 0) l = r = 0;
            else if (i % 8 == 1) l = r = n;
            else if (i % 8 == 2) { l = 0; r = n; }
            else if (i % 8 == 3) { l = n - 1; r = n; }
        }
        input.ranges.emplace_back(l, r);
    }
    return input;
}
std::uint64_t reference(const Input& input) {
    if (input.pattern == 2) {
        std::uint64_t checksum = 0;
        for (auto [l, limit] : input.ranges)
            checksum = ::sirkit::detail::benchmark_digest(checksum,
                std::min(limit, l < input.text.size() ? input.text.size() - l - 1 : 0));
        return checksum;
    }
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
    if (input.pattern == 2) {
        RollingHash other(input.other, base);
        for (auto [l, limit] : input.ranges)
            checksum = ::sirkit::detail::benchmark_digest(checksum, hash.lcp(l, other, l, limit));
        return checksum;
    }
    for (auto [l, r] : input.ranges)
        checksum = ::sirkit::detail::benchmark_digest(checksum, hash.get(l, r));
    return checksum;
}
}
int main(int argc, char** argv) {
    return ::sirkit::detail::benchmark_main(argc, argv, ::sirkit::detail::benchmark_rolling_hash::generate,
        ::sirkit::detail::benchmark_rolling_hash::reference, ::sirkit::detail::benchmark_rolling_hash::kernel);
}
