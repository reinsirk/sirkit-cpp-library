// SPDX-License-Identifier: CC0-1.0
#include "benchmark_common.hpp"
#include "library.hpp"
#include <algorithm>
#include <vector>

namespace sirkit::detail::benchmark_fenwick {
struct Input {
    std::vector<std::int64_t> values;
    std::vector<std::pair<int, int>> ranges;
};
Input generate(std::size_t n, std::size_t pattern) {
    std::uint64_t seed = 20261114;
    Input input;
    input.values.resize(n);
    for (auto& value : input.values)
        value = static_cast<std::int64_t>(::sirkit::detail::benchmark_random(seed) % 2001) - 1000;
    input.ranges.reserve(n * 4);
    for (std::size_t i = 0; i < n * 4; ++i) {
        auto l = static_cast<int>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        auto r = static_cast<int>(::sirkit::detail::benchmark_random(seed) % (n + 1));
        if (pattern) { l = 0; r = static_cast<int>(i % n + 1); }
        if (l > r) std::swap(l, r);
        input.ranges.emplace_back(l, r);
    }
    return input;
}
std::uint64_t reference(const Input& input) {
    std::vector<std::int64_t> prefix(input.values.size() + 1);
    for (std::size_t i = 0; i < input.values.size(); ++i) prefix[i + 1] = prefix[i] + input.values[i];
    std::uint64_t checksum = 0;
    for (auto [l, r] : input.ranges)
        checksum = ::sirkit::detail::benchmark_digest(checksum, static_cast<std::uint64_t>(prefix[r] - prefix[l]));
    return checksum;
}
std::uint64_t kernel(const Input& input) {
    fenwick_tree<std::int64_t> tree(static_cast<int>(input.values.size()));
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
