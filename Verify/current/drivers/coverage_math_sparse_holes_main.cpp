// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

// 提出先のACL有無に依存しない、このドライバ専用の法998244353整数。
struct VerifyMod998 {
    std::uint32_t value = 0;
    VerifyMod998() = default;
    VerifyMod998(std::int64_t x) {
        x %= 998244353;
        if (x < 0) x += 998244353;
        value = static_cast<std::uint32_t>(x);
    }
    VerifyMod998& operator+=(VerifyMod998 b) { value += b.value; if (value >= 998244353) value -= 998244353; return *this; }
    VerifyMod998& operator-=(VerifyMod998 b) { if (value < b.value) value += 998244353; value -= b.value; return *this; }
    VerifyMod998& operator*=(VerifyMod998 b) { value = static_cast<std::uint32_t>(std::uint64_t(value) * b.value % 998244353); return *this; }
    friend VerifyMod998 operator+(VerifyMod998 a, VerifyMod998 b) { return a += b; }
    friend VerifyMod998 operator-(VerifyMod998 a, VerifyMod998 b) { return a -= b; }
    friend VerifyMod998 operator*(VerifyMod998 a, VerifyMod998 b) { return a *= b; }
    friend bool operator==(VerifyMod998, VerifyMod998) = default;
};

// gcd(2i,2j)=2gcd(i,j)。全約数閉包には大きい奇数の穴が必ず残る。
int main() {
    int n{}; verify_in >> n;
    std::vector<std::pair<int, VerifyMod998>> left, right;
    const auto key_count = n + (n + 1) / 2;
    left.reserve(key_count); right.reserve(key_count);
    for (int key = 1; key <= n; ++key) {
        left.emplace_back(key, VerifyMod998{});
        right.emplace_back(key, VerifyMod998{});
    }
    for (int key = (n / 2 + 1) * 2; key <= 2 * n; key += 2) {
        left.emplace_back(key, VerifyMod998{});
        right.emplace_back(key, VerifyMod998{});
    }
    const auto read_values = [&](auto& values) {
        for (int i = 1; i <= n / 2; ++i) verify_in >> values[2 * i - 1].second.value;
        for (int i = n; i < key_count; ++i) verify_in >> values[i].second.value;
    };
    read_values(left); read_values(right);
    IntegerSieve sieve(2 * n);
    sparse_multiple_zeta_transform(left, sieve); sparse_multiple_zeta_transform(right, sieve);
    for (std::size_t i = 0; i < left.size(); ++i) left[i].second *= right[i].second;
    sparse_multiple_mobius_transform(left, sieve);
    for (const auto& [key, value] : left) if (key % 2 == 0) verify_out << value.value << ' ';
    verify_out << '\n';
}
