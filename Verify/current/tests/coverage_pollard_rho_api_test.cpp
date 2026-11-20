// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Pollard_Rho.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cfenv>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

using Uint = std::uint64_t;
using Factors = std::vector<Uint>;
using Powers = std::vector<std::pair<Uint, int>>;

Factors trial_factors(Uint remaining) {
    Factors factors;
    for (Uint divisor = 2; divisor <= remaining / divisor; ++divisor) {
        while (remaining % divisor == 0) {
            factors.push_back(divisor);
            remaining /= divisor;
        }
    }
    if (remaining > 1) factors.push_back(remaining);
    return factors;
}

template <class Function>
void rejects_zero(Function function) {
    try {
        function();
    } catch (const std::domain_error&) {
        return;
    }
    assert(false);
}

void check_divisor_functions(sirkit::PollardRho64& factorizer, Uint number, int mobius) {
    Factors divisors;
    Uint coprime = 0;
    for (Uint value = 1; value <= number; ++value) {
        if (number % value == 0) divisors.push_back(value);
        if (std::gcd(value, number) == 1) ++coprime;
    }
    assert(factorizer.divisors(number) == divisors);
    auto enumerated = factorizer.divisors(number, false);
    std::sort(enumerated.begin(), enumerated.end());
    assert(enumerated == divisors);
    assert(factorizer.divisor_count(number) == divisors.size());
    assert(factorizer.totient(number) == coprime);
    assert(factorizer.mobius(number) == mobius);
}

void check_large_integers() {
    constexpr Uint p = 4'294'967'279;
    constexpr Uint q = 4'294'967'291;
    assert(trial_factors(p) == Factors{p});
    assert(trial_factors(q) == Factors{q});
    const Uint semiprime = p * q;
    const Factors factors{p, q};
    const Powers powers{{p, 1}, {q, 1}};
    const Factors divisors{1, p, q, semiprime};
    const Powers square_powers{{q, 2}};
    const Factors maximum_factors{3, 5, 17, 257, 641, 65537, 6700417};
    const Factors below_square_factors{9241, 464773, q};

    sirkit::PollardRho64 factorizer(1);
    assert(factorizer.prime_factors(semiprime) == factors);
    assert(factorizer.factorize(q * q) == square_powers);
    // 平方数の直前では浮動小数点の平方根が整数へ丸められる場合がある。
    assert(factorizer.prime_factors(q * (q + 2)) == below_square_factors);
#ifdef FE_UPWARD
    const int original_rounding = std::fegetround();
    if (original_rounding != -1 && std::fesetround(FE_UPWARD) == 0) {
        const auto rounded_factors = factorizer.prime_factors(q * (q + 2));
        const int restored = std::fesetround(original_rounding);
        assert(restored == 0);
        assert(rounded_factors == below_square_factors);
    }
#endif
    assert(factorizer.prime_factors(std::numeric_limits<Uint>::max()) == maximum_factors);
    assert(sirkit::PollardRho64::is_prime(q));
    assert(!sirkit::PollardRho64::is_prime(q * q));
    assert(!sirkit::PollardRho64::is_prime(3'825'123'056'546'413'051));
    assert(sirkit::PrimeFactors64(semiprime) == factors);
    assert(sirkit::Factorize64(semiprime) == powers);
    assert(sirkit::Divisors64(semiprime) == divisors);
    assert(sirkit::DivisorCount64(semiprime) == divisors.size());
    assert(sirkit::EulerPhi64(semiprime) == (p - 1) * (q - 1));
    assert(sirkit::Mobius64(semiprime) == 1);

    // 初回の因数探索が予算内に終わらなくても、乱数を更新して結果を得る。
    constexpr Uint first = 2'147'483'123, second = 2'147'483'579;
    assert(trial_factors(first) == Factors{first});
    assert(trial_factors(second) == Factors{second});
    const Powers retry_factors{{first, 1}, {second, 1}};
    assert(sirkit::PollardRho64(9'387'825'707'080'321'328ULL).factorize(first * second) == retry_factors);
}

} // namespace

int main() {
    sirkit::PollardRho64 factorizer;
    assert(!sirkit::PollardRho64::is_prime(0));
    assert(!sirkit::PollardRho64::is_prime(1));
    assert(sirkit::PollardRho64::is_prime(37));
    assert(!sirkit::PollardRho64::is_prime(74));
    // 73は判定に使う底を割り、5461は底2のFermat擬素数。
    assert(sirkit::PollardRho64::is_prime(73));
    assert(!sirkit::PollardRho64::is_prime(5461));
    assert(factorizer.prime_factors(1).empty());
    assert(factorizer.factorize(1).empty());
    check_divisor_functions(factorizer, 1, 1);
    check_divisor_functions(factorizer, 41, -1);
    check_divisor_functions(factorizer, 41 * 43, 1);
    check_divisor_functions(factorizer, 41 * 41, 0);
    check_divisor_functions(factorizer, 3 * 3 * 5 * 7, 0);

    // 同じ合成数を複数の乱数列で照合し、分割の経路を変える。
    constexpr std::array<Uint, 5> composites{41 * 43, 43 * 47, 47 * 53, 53 * 59, 41 * 43 * 47};
    for (Uint value : composites) {
        const auto expected = trial_factors(value);
        for (Uint seed = 0; seed < 16; ++seed) {
            assert(sirkit::PollardRho64(seed).prime_factors(value) == expected);
        }
    }
    check_large_integers();

    rejects_zero([&] { (void)factorizer.prime_factors(0); });
    rejects_zero([&] { (void)factorizer.factorize(0); });
    rejects_zero([&] { (void)factorizer.divisors(0); });
    rejects_zero([&] { (void)factorizer.divisor_count(0); });
    rejects_zero([&] { (void)factorizer.totient(0); });
    rejects_zero([&] { (void)factorizer.mobius(0); });
    rejects_zero([] { (void)sirkit::PrimeFactors64(0); });
    rejects_zero([] { (void)sirkit::Factorize64(0); });
    rejects_zero([] { (void)sirkit::Divisors64(0); });
    rejects_zero([] { (void)sirkit::DivisorCount64(0); });
    rejects_zero([] { (void)sirkit::EulerPhi64(0); });
    rejects_zero([] { (void)sirkit::Mobius64(0); });
    assert(factorizer.prime_factors(41 * 43) == trial_factors(41 * 43));
}
