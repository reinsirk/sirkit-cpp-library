// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>
#include <vector>
#ifndef SIRKIT_LIS_SOURCE
#define SIRKIT_LIS_SOURCE "../../../Algorithm/LIS.cpp"
#endif
#include SIRKIT_LIS_SOURCE

static std::uintmax_t cases = 0, checks = 0, phased = 0, avx = 0;
static void require(bool ok) { ++checks; if (!ok) std::abort(); }

// 座標圧縮とFenwick最大値DP。末尾配列の探索とは独立に長さを求める。
static std::size_t reference(const std::vector<std::int64_t>& input) {
    auto keys = input;
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    std::vector<std::size_t> bit(keys.size() + 1);
    std::size_t result = 0;
    for (auto value : input) {
        const auto rank = static_cast<std::size_t>(std::lower_bound(keys.begin(), keys.end(), value) - keys.begin());
        std::size_t best = 0;
        for (auto at = rank; at != 0; at &= at - 1) best = std::max(best, bit[at]);
        ++best;
        result = std::max(result, best);
        for (auto at = rank + 1; at < bit.size(); at += at & (~at + 1)) bit[at] = std::max(bit[at], best);
    }
    return result;
}
static void validate(const std::vector<std::int64_t>& input, const std::vector<std::size_t>& indices, std::size_t expected) {
    require(indices.size() == expected);
    for (std::size_t i = 0; i < indices.size(); ++i) {
        require(indices[i] < input.size());
        if (i != 0) {
            require(indices[i - 1] < indices[i]);
            require(input[indices[i - 1]] < input[indices[i]]);
        }
    }
}
static void run(const std::vector<std::int64_t>& input) {
    ++cases;
    const auto saved = input;
    const auto expected = reference(input);
    validate(input, LISIndices(input), expected);
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 15 && __GNUC_MINOR__ == 2 && defined(__x86_64__)
    validate(input, reinsirk::detail::LISIndicesPhased<std::uint32_t>(input), expected);
    validate(input, reinsirk::detail::LISIndicesPhased<std::size_t>(input), expected);
    phased += 2;
    if (__builtin_cpu_supports("avx2")) {
        validate(input, reinsirk::detail::LISIndicesWithValuesAVXTail<std::uint32_t>(input), expected);
        validate(input, reinsirk::detail::LISIndicesWithValuesAVXTail<std::size_t>(input), expected);
        avx += 2;
    }
#endif
    require(saved == input);
}
static std::vector<std::int64_t> ascending(std::size_t length) {
    std::vector<std::int64_t> v;
    for (std::size_t i = 0; i < length; ++i) v.push_back(static_cast<std::int64_t>(4 * i));
    return v;
}
static std::vector<std::int64_t> searches(std::size_t length, std::size_t prefix = 0) {
    std::vector<std::int64_t> v(prefix, 0);
    const auto a = ascending(length);
    v.insert(v.end(), a.begin(), a.end());
    std::vector<std::size_t> positions{1, length / 4, length / 2, length - 2, length - 1};
    if (length < 64) for (std::size_t i = 1; i < length; ++i) positions.push_back(i);
    for (auto at : positions) if (at != 0 && at < length)
        for (std::int64_t delta : {-1, 0, 1}) {
            const auto key = static_cast<std::int64_t>(4 * at) + delta;
            if (key < a.back()) v.push_back(key);
        }
    return v;
}
int main() {
    run({}); run({0}); run({0, 0}); run({1, 0});
    const auto low = std::numeric_limits<std::int64_t>::min();
    const auto high = std::numeric_limits<std::int64_t>::max();
    run({low, high, low, 0, high, -1, 1, high, low});
    for (std::size_t length : {33u, 34u, 35u, 4098u, 4099u, 4100u}) {
        auto v = ascending(length);
        run(v); // phase遷移直後が入力末尾になる場合も含む。
        v.push_back(static_cast<std::int64_t>(2 * length));
        run(v); // 遷移の次の要素を探索する。
    }
    for (std::size_t count : {3u, 4u, 7u, 8u, 15u, 16u, 31u, 32u, 33u, 4096u, 4097u, 4098u}) {
        const auto length = count + 2;
        auto v = searches(length, length > 4096 ? 20000 : 0);
        run(v);
        for (auto& value : v) value += low; // 全値は非負かつ小さいので安全。
        run(v);
        for (auto& value : v) value = high - (static_cast<std::int64_t>(4 * (length - 1)) - (value - low));
        run(v);
    }
    for (std::size_t n : {65535u, 65536u, 65537u}) for (std::size_t length : {34u, 4100u}) {
        std::vector<std::int64_t> v;
        // 最長境界の1例では先読み長>64とし、phase選択も公開APIから通す。
        for (std::size_t i = 0; i < 256; ++i) {
            const auto prefix_value = n == 65537 && length == 4100 ? i : i % 32;
            v.push_back(static_cast<std::int64_t>(4 * prefix_value));
        }
        const auto body = searches(length, 20000);
        v.insert(v.end(), body.begin(), body.end());
        std::uint64_t state = 0x45d9f3b;
        while (v.size() < n) {
            state ^= state << 13; state ^= state >> 7; state ^= state << 17;
            v.push_back(std::bit_cast<std::int64_t>(state));
        }
        run(v);
    }
    std::printf("PASS phase reference cases=%ju checks=%ju phased=%ju avx=%ju\n", cases, checks, phased, avx);
}
