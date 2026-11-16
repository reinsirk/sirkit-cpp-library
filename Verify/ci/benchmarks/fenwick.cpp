// SPDX-License-Identifier: CC0-1.0
#include "benchmark_common.hpp"
#include "library.hpp"
#include <algorithm>
#include <vector>

namespace sirkit::detail::benchmark_fenwick {
struct Input {
    std::size_t pattern = 0;
    std::vector<std::int64_t> values;
    std::vector<std::pair<int, int>> ranges;
};
Input generate(std::size_t n, std::size_t pattern) {
    std::uint64_t seed = 20261114;
    Input input;
    input.pattern = pattern;
    input.values.resize(n);
    if (pattern == 3) return input;
    for (auto& value : input.values)
        value = static_cast<std::int64_t>(::sirkit::detail::benchmark_random(seed) % 2001) - 1000;
    input.ranges.reserve(n * 4);
    std::size_t worst_prefix = 0;
    while (worst_prefix <= (n - 1) / 2) worst_prefix = worst_prefix * 2 + 1;
    for (std::size_t i = 0; i < n * 4; ++i) {
        auto l = static_cast<int>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        auto r = static_cast<int>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        if (pattern) { l = 0; r = static_cast<int>(i % n + 1); }
        if (pattern == 2) {
            switch (i % 6) {
            case 0: l = r = 0; break;
            case 1: l = r = static_cast<int>(n); break;
            case 2: l = 0; r = static_cast<int>(n); break;
            case 3: l = static_cast<int>(n - 1); r = static_cast<int>(n); break;
            case 4: l = 0; r = 1; break;
            default:
                l = 0; r = static_cast<int>(worst_prefix); break;
            }
        }
        if (l > r) std::swap(l, r);
        input.ranges.emplace_back(l, r);
    }
    return input;
}
std::uint64_t reference(const Input& input) {
    if (input.pattern == 3) {
        // 左から1ずつ追加し、右から取り消す。各時点の総和は閉形式。
        std::uint64_t checksum = 0;
        for (std::size_t i = 1; i <= input.values.size(); ++i)
            checksum = ::sirkit::detail::benchmark_digest(checksum, i);
        for (std::size_t i = input.values.size(); i > 0; --i)
            checksum = ::sirkit::detail::benchmark_digest(checksum, i - 1);
        return checksum;
    }
    std::vector<std::int64_t> prefix(input.values.size() + 1);
    for (std::size_t i = 0; i < input.values.size(); ++i) prefix[i + 1] = prefix[i] + input.values[i];
    std::uint64_t checksum = 0;
    for (auto [l, r] : input.ranges)
        checksum = ::sirkit::detail::benchmark_digest(checksum, static_cast<std::uint64_t>(prefix[r] - prefix[l]));
    return checksum;
}
std::uint64_t kernel(const Input& input) {
    fenwick_tree<std::int64_t> tree(static_cast<int>(input.values.size()));
    if (input.pattern == 3) {
        std::uint64_t checksum = 0;
        for (int i = 0; i < tree.size(); ++i) {
            tree.add(i, 1);
            checksum = ::sirkit::detail::benchmark_digest(checksum, tree.sum(0, i + 1));
        }
        for (int i = tree.size(); i > 0; --i) {
            tree.add(i - 1, -1);
            checksum = ::sirkit::detail::benchmark_digest(checksum, tree.sum(0, tree.size()));
        }
        return checksum;
    }
    for (int i = 0; i < static_cast<int>(input.values.size()); ++i) tree.add(i, input.values[i]);
    std::uint64_t checksum = 0;
    for (auto [l, r] : input.ranges)
        checksum = ::sirkit::detail::benchmark_digest(checksum, static_cast<std::uint64_t>(tree.sum(l, r)));
    return checksum;
}
}
int main(int argc, char** argv) {
    return ::sirkit::detail::benchmark_main(argc, argv, ::sirkit::detail::benchmark_fenwick::generate,
        ::sirkit::detail::benchmark_fenwick::reference, ::sirkit::detail::benchmark_fenwick::kernel);
}
