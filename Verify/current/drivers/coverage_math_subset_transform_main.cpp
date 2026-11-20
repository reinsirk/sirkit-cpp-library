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
    VerifyMod998& operator+=(VerifyMod998 b) { const auto sum = value + b.value; value = sum - (sum >= 998244353 ? 998244353 : 0); return *this; }
    VerifyMod998& operator-=(VerifyMod998 b) { const auto difference = value + 998244353 - b.value; value = difference - (difference >= 998244353 ? 998244353 : 0); return *this; }
    VerifyMod998& operator*=(VerifyMod998 b) { value = static_cast<std::uint32_t>(std::uint64_t(value) * b.value % 998244353); return *this; }
    friend VerifyMod998 operator+(VerifyMod998 a, VerifyMod998 b) { return a += b; }
    friend VerifyMod998 operator-(VerifyMod998 a, VerifyMod998 b) { return a -= b; }
    friend VerifyMod998 operator*(VerifyMod998 a, VerifyMod998 b) { return a *= b; }
    friend bool operator==(VerifyMod998, VerifyMod998) = default;
};

// 補集合で添字を反転するとAND畳み込みはOR畳み込みになる。
int main() {
    int exponent{}; verify_in >> exponent;
    const std::size_t n = std::size_t{1} << exponent;
    std::vector<VerifyMod998> a(n), b(n);
    for (std::size_t i = n; i-- > 0;) verify_in >> a[i].value;
    for (std::size_t i = n; i-- > 0;) verify_in >> b[i].value;
    subset_zeta_transform(a); subset_zeta_transform(b);
    for (std::size_t i = 0; i < n; ++i) a[i] *= b[i];
    subset_mobius_transform(a);
    for (std::size_t i = n; i-- > 0;) verify_out << a[i].value << ' ';
    verify_out << '\n';
}
