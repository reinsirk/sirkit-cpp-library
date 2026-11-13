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

int main() {
    int n{}; verify_in >> n;
    IntegerSieve sieve(n);
    std::vector<std::pair<int, VerifyMod998>> a(n), b(n);
    for (int i = 1; i <= n; ++i) { a[i - 1].first = i; verify_in >> a[i - 1].second.value; }
    for (int i = 1; i <= n; ++i) { b[i - 1].first = i; verify_in >> b[i - 1].second.value; }
    sparse_multiple_zeta_transform(a, sieve); sparse_multiple_zeta_transform(b, sieve);
    for (int i = 0; i < n; ++i) a[i].second *= b[i].second;
    sparse_multiple_mobius_transform(a, sieve);
    for (int i = 0; i < n; ++i) verify_out << a[i].second.value << ' ';
    verify_out << '\n';
}
