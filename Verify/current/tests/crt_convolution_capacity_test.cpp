// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
// 最大変換長とCRT復元の補足試験。大きい領域を使うため通常の使用例とは分ける。
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>
#ifndef HEADER
#define HEADER "../../../Math/Convolution.cpp"
#endif
#include HEADER
int main(int argc, char** argv) {
    if (argc != 2) throw std::invalid_argument("choose modular or integer");
    constexpr std::size_t length = std::size_t{1} << 23;
    CRTConvolution convolution;
    if (std::string_view(argv[1]) == "modular") {
        constexpr std::size_t n = length / 2, m = n + 1;
        constexpr std::uint32_t modulus = UINT32_MAX, value = modulus - 1;
        const std::vector<std::uint32_t> a(n, value), b(m, value);
        const auto result = convolution.modular(a, b, modulus);
        if (result.size() != length) throw std::runtime_error("maximum modular size");
        for (std::size_t i = 0; i < result.size(); ++i) {
            // (-1)*(-1)=1。三角形の項数だけで独立に照合する。
            const auto count = std::min({i + 1, result.size() - i, n, m});
            if (result[i] != count) throw std::runtime_error("maximum modular coefficient");
        }
        std::cout << "PASS max-length modular: " << length << " coefficients\n";
    } else if (std::string_view(argv[1]) == "integer") {
        constexpr std::size_t m = 33, n = length - m + 1;
        constexpr auto value = std::numeric_limits<std::int64_t>::max();
        std::vector<std::int64_t> a(n);
        const std::vector<std::int64_t> b(m, 1);
        for (std::size_t i = 0; i < n; ++i) a[i] = i % 2 ? -value : value;
        const auto result = convolution.integer(a, b);
        if (result.size() != length) throw std::runtime_error("maximum integer size");
        for (std::size_t i = 0; i < result.size(); ++i) {
            // 連続する交互符号列。偶数項なら0、奇数項なら先頭の符号だけが残る。
            const auto first = i < m ? 0 : i - m + 1;
            const auto last = std::min(i, n - 1);
            const auto count = last - first + 1;
            const auto expected = count % 2 ? (first % 2 ? -value : value) : 0;
            if (result[i] != expected) throw std::runtime_error("maximum signed coefficient");
        }
        std::cout << "PASS max-length signed cancellation: " << length << " coefficients\n";
    } else throw std::invalid_argument("unknown capacity test");
}
