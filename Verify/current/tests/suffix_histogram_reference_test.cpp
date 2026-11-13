// SPDX-License-Identifier: CC0-1.0
#ifndef SUFFIX_HEADER
#define SUFFIX_HEADER "../../../String/Suffix_Array.cpp"
#endif
#include SUFFIX_HEADER
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

namespace {
std::uint64_t checks = 0;
void check(bool value) { ++checks; if (!value) std::abort(); }
}

int main() {
    std::mt19937_64 rng(416499);
    for (int n : {0, 1, 2, 7, 4095, 4096, 4097, 4098, 4099, 4100, 4101, 4102, 500000}) {
        for (int shape = 0; shape < 6; ++shape) {
            std::vector<int> a(n), expected;
            int upper = 0;
            if (shape < 3) {
                std::iota(a.begin(), a.end(), 0);
                if (shape == 1) std::reverse(a.begin(), a.end());
                if (shape == 2) std::shuffle(a.begin(), a.end(), rng);
                upper = std::max(0, n - 1);
                expected.resize(n);
                // 最初の値がすべて異なるので、その値だけで接尾辞順が決まる。
                for (int i = 0; i < n; ++i) expected[a[i]] = i;
            } else {
                const int period = shape == 3 ? 1 : shape == 4 ? 256 : 257;
                upper = period - 1;
                for (int i = 0; i < n; ++i) a[i] = i % period;
                // 同じ開始値の周期列は短い接尾辞から並ぶ。
                for (int first = 0; first < period && first < n; ++first) {
                    const int last = n - 1 - (n - 1 - first) % period;
                    for (int i = last; i >= first; i -= period) expected.push_back(i);
                }
            }
            std::vector<std::int64_t> wide(n);
            for (int i = 0; i < n; ++i)
                wide[i] = std::int64_t{a[i]} * 1000000007 - 5000000000000;
            const auto bounded = suffix_array(a, upper);
            check(bounded == expected);
            check(suffix_array(wide) == expected);
            const auto lcp = lcp_array(a, bounded);
            check(lcp.size() == (n == 0 ? 0 : static_cast<std::size_t>(n - 1)));
            for (int i = 0; i + 1 < n; ++i) {
                const int x = bounded[i], y = bounded[i + 1];
                const int length = a[x] == a[y] ? n - std::max(x, y) : 0;
                check(lcp[i] == length);
            }
        }
    }
    std::printf("histogram reference checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
