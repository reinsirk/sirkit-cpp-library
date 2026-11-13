// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Pollard_Rho.cpp
// 参考（数学的手法。実装は独自構成）:
// https://doi.org/10.1007/BF01933190
// https://miller-rabin.appspot.com/
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pollard_rho.md
//
// API一覧
// PollardRho64(seed=定数): 乱数状態だけを持つ64bit因数分解器。コピーは独立、各操作は入力整数を変更しない。
// is_prime(n): uint64_t全域の素数判定。底7個の決定的Miller-Rabin、O(log n)回の剰余乗算。
// prime_factors(n): 素因数を重複込みの昇順で返す。factorize(n): {素数,指数}を昇順で返す。
// divisors(n,sorted=true): 正の約数。sorted=falseは列挙順、trueは昇順。divisor_count(n): 約数の個数。
// totient(n)/mobius(n): Eulerのφ/メビウス関数を返す。n=1では因数{}・約数{1}・φ=1・μ=1。
// 因数分解・約数・φ・μはn>=1が必要。n=0は検査省略時もdomain_error。
// 因数分解は乱択Brent法で、素因数pを分離する期待O(sqrt(p))回の剰余乗算。確定的時間上界は保証しない。
// 分解後、約数列挙はD個にO(D)、整列ありはO(D log D)。他は素因数の個数に比例。
// PrimeFactors64/Factorize64/Divisors64/DivisorCount64/EulerPhi64/Mobius64(n): 一時分解器を使う同名操作。
// 128bit拡張を使えるGCC/Clangでは高速演算、それ以外は標準C++の32bit limb演算。
// SIRKIT_FORCE_STANDARD_ARITHMETICで拡張側を除外できる。既存is_prime/MillerRabinとは部品・guardが独立。

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_POLLARD_RHO_DETAIL_DEFINED
#define SIRKIT_POLLARD_RHO_DETAIL_DEFINED
namespace reinsirk::detail {
class FactorizationArithmetic {
    std::uint64_t inverse_, r_squared_;
public:
    std::uint64_t modulus, one;
    explicit FactorizationArithmetic(std::uint64_t n) noexcept : modulus(n) {
        std::uint64_t inverse = n;
        for (int i = 0; i < 6; ++i) inverse *= 2 - n * inverse;
        inverse_ = std::uint64_t{0} - inverse;
        const auto residue = std::numeric_limits<std::uint64_t>::max() % n;
        one = residue == n - 1 ? 0 : residue + 1;
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
        using Wide = __uint128_t; // GCC/Clang拡張。標準C++の型ではない。
        r_squared_ = static_cast<std::uint64_t>(static_cast<Wide>(one) * one % n);
#else
        r_squared_ = one;
        for (int i = 0; i < 64; ++i) r_squared_ = add(r_squared_, r_squared_);
#endif
    }
    [[nodiscard]] std::uint64_t add(std::uint64_t a, std::uint64_t b) const noexcept {
        return a >= modulus - b ? a - (modulus - b) : a + b;
    }
    [[nodiscard]] std::uint64_t multiply(std::uint64_t a, std::uint64_t b) const noexcept {
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
        using Wide = __uint128_t;
        const Wide product = static_cast<Wide>(a) * b;
        const auto q = static_cast<std::uint64_t>(product) * inverse_;
        const Wide correction = static_cast<Wide>(q) * modulus;
        // 128bit全体を加算せず、65bitの上位和で129bit目の繰上がりを保持する。
        const Wide upper = (product >> 64) + (correction >> 64) + (static_cast<std::uint64_t>(product) != 0);
        return static_cast<std::uint64_t>(upper >= modulus ? upper - modulus : upper);
#else
        constexpr std::uint64_t mask = (std::uint64_t{1} << 32) - 1;
        const std::uint32_t a0 = static_cast<std::uint32_t>(a), a1 = static_cast<std::uint32_t>(a >> 32);
        const std::uint32_t b0 = static_cast<std::uint32_t>(b), b1 = static_cast<std::uint32_t>(b >> 32);
        const auto p00 = std::uint64_t{a0} * b0;
        auto middle = std::uint64_t{a0} * b1 + (p00 >> 32);
        const auto upper_carry = middle >> 32;
        middle = std::uint64_t{a1} * b0 + (middle & mask);
        const auto high = std::uint64_t{a1} * b1 + upper_carry + (middle >> 32);
        std::array<std::uint32_t, 5> words{static_cast<std::uint32_t>(p00),
            static_cast<std::uint32_t>(middle), static_cast<std::uint32_t>(high),
            static_cast<std::uint32_t>(high >> 32), 0};
        const std::uint32_t mod0 = static_cast<std::uint32_t>(modulus), mod1 = static_cast<std::uint32_t>(modulus >> 32);
        for (std::size_t i = 0; i < 2; ++i) {
            const auto q = static_cast<std::uint32_t>(std::uint64_t{words[i]} * static_cast<std::uint32_t>(inverse_));
            auto sum = std::uint64_t{q} * mod0 + words[i];
            words[i] = static_cast<std::uint32_t>(sum);
            sum = std::uint64_t{q} * mod1 + words[i + 1] + (sum >> 32);
            words[i + 1] = static_cast<std::uint32_t>(sum);
            std::uint64_t carry = sum >> 32;
            for (std::size_t at = i + 2; carry != 0; ++at) {
                const auto combined = std::uint64_t{words[at]} + carry;
                words[at] = static_cast<std::uint32_t>(combined); carry = combined >> 32;
            }
        }
        const auto result = (std::uint64_t{words[3]} << 32) | words[2];
        return words[4] != 0 || result >= modulus ? result - modulus : result;
#endif
    }
    [[nodiscard]] std::uint64_t encode(std::uint64_t value) const noexcept {
        return multiply(value % modulus, r_squared_);
    }
    [[nodiscard]] std::uint64_t power(std::uint64_t value, std::uint64_t exponent) const noexcept {
        auto result = one;
        for (; exponent != 0; exponent >>= 1) {
            if (exponent & 1) result = multiply(result, value);
            value = multiply(value, value);
        }
        return result;
    }
    [[nodiscard]] bool prime() const noexcept {
        const int shifts = std::countr_zero(modulus - 1);
        const auto exponent = (modulus - 1) >> shifts;
        for (std::uint64_t base : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
            if (base % modulus == 0) continue;
            auto value = power(encode(base), exponent);
            if (value == one || value == modulus - one) continue;
            int step = 1;
            for (; step < shifts; ++step) {
                value = multiply(value, value);
                if (value == modulus - one) break;
                if (value == one) return false;
            }
            if (step == shifts) return false;
        }
        return true;
    }
};
inline constexpr std::array<std::uint32_t, 12> factor_small_primes{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
inline std::uint64_t factor_square_root(std::uint64_t value) noexcept {
    auto root = static_cast<std::uint64_t>(std::sqrt(static_cast<long double>(value)));
    while (root != 0 && root > value / root) --root;
    while (root + 1 <= value / (root + 1)) ++root;
    return root;
}
}
#endif

#ifndef SIRKIT_POLLARD_RHO64_DEFINED
#define SIRKIT_POLLARD_RHO64_DEFINED
class PollardRho64 {
    std::uint64_t state_;
    std::uint64_t random_below(std::uint64_t upper) noexcept {
        state_ = state_ * std::uint64_t{6364136223846793005} + std::uint64_t{1442695040888963407};
        return state_ % upper;
    }
    std::uint64_t split(const reinsirk::detail::FactorizationArithmetic& arithmetic) noexcept {
        const auto n = arithmetic.modulus;
        constexpr std::size_t batch = 128, attempt_budget = std::size_t{1} << 22;
        for (;;) {
            const auto constant = arithmetic.encode(random_below(n - 1) + 1);
            auto moving = arithmetic.encode(random_below(n - 2) + 2);
            const auto advance = [&](std::uint64_t value) { return arithmetic.add(arithmetic.multiply(value, value), constant); };
            std::uint64_t fixed = moving, saved = moving, divisor = 1;
            std::size_t span = 1, spent = 0;
            while (divisor == 1 && span <= (attempt_budget - spent) / 2) {
                fixed = moving;
                for (std::size_t i = 0; i < span; ++i) moving = advance(moving);
                spent += span;
                for (std::size_t offset = 0; offset < span && divisor == 1; offset += batch) {
                    saved = moving;
                    auto product = arithmetic.one;
                    const auto count = std::min(batch, span - offset);
                    for (std::size_t j = 0; j < count; ++j) {
                        moving = advance(moving);
                        const auto difference = fixed >= moving ? fixed - moving : moving - fixed;
                        product = arithmetic.multiply(product, difference);
                    }
                    spent += count;
                    divisor = std::gcd(product, n);
                }
                span <<= 1;
            }
            if (divisor == n) {
                for (std::size_t i = 0; i < batch; ++i) {
                    saved = advance(saved);
                    const auto difference = fixed >= saved ? fixed - saved : saved - fixed;
                    divisor = std::gcd(difference, n);
                    if (divisor > 1) break;
                }
            }
            if (divisor > 1 && divisor < n) return divisor;
        }
    }
    static void require_positive(std::uint64_t n) {
        if (n == 0) throw std::domain_error("PollardRho64: zero has no finite factorization");
    }
public:
    explicit PollardRho64(std::uint64_t seed = 0x8b54e7163d20a9c1ULL) noexcept : state_(seed) {}
    [[nodiscard]] static bool is_prime(std::uint64_t n) noexcept {
        if (n < 2) return false;
        for (auto prime : reinsirk::detail::factor_small_primes)
            if (n % prime == 0) return n == prime;
        return reinsirk::detail::FactorizationArithmetic(n).prime();
    }
    std::vector<std::uint64_t> prime_factors(std::uint64_t n) {
        require_positive(n);
        if (n == 1) return {};
        std::vector<std::uint64_t> result; result.reserve(64);
        for (auto prime : reinsirk::detail::factor_small_primes)
            while (n % prime == 0) { result.push_back(prime); n /= prime; }
        if (n == 1) return result;
        std::vector<std::pair<std::uint64_t, int>> pending;
        pending.reserve(64);
        if (n != 1) pending.emplace_back(n, 1);
        while (!pending.empty()) {
            const auto [value, multiplicity] = pending.back(); pending.pop_back();
            const reinsirk::detail::FactorizationArithmetic arithmetic(value);
            if (arithmetic.prime()) {
                for (int i = 0; i < multiplicity; ++i) result.push_back(value);
                continue;
            }
            const auto root = reinsirk::detail::factor_square_root(value);
            if (root > 1 && root == value / root && value % root == 0) {
                pending.emplace_back(root, multiplicity * 2); continue;
            }
            const auto divisor = split(arithmetic);
            pending.emplace_back(divisor, multiplicity);
            pending.emplace_back(value / divisor, multiplicity);
        }
        std::sort(result.begin(), result.end());
        return result;
    }
    std::vector<std::pair<std::uint64_t, int>> factorize(std::uint64_t n) {
        const auto primes = prime_factors(n);
        std::vector<std::pair<std::uint64_t, int>> result;
        for (auto prime : primes) {
            if (result.empty() || result.back().first != prime) result.emplace_back(prime, 1);
            else ++result.back().second;
        }
        return result;
    }
    std::vector<std::uint64_t> divisors(std::uint64_t n, bool sorted = true) {
        const auto powers = factorize(n);
        std::vector<std::uint64_t> result{1};
        std::size_t count = 1;
        for (const auto& [prime, exponent] : powers) {
            (void)prime;
            if (count > result.max_size() / static_cast<std::size_t>(exponent + 1))
                throw std::length_error("PollardRho64: too many divisors");
            count *= static_cast<std::size_t>(exponent + 1);
        }
        result.reserve(count);
        for (const auto& [prime, exponent] : powers) {
            const auto previous = result.size();
            std::uint64_t power = 1;
            for (int i = 0; i < exponent; ++i) {
                power *= prime;
                for (std::size_t j = 0; j < previous; ++j) result.push_back(result[j] * power);
            }
        }
        if (sorted) std::sort(result.begin(), result.end());
        return result;
    }
    std::uint64_t divisor_count(std::uint64_t n) {
        std::uint64_t result = 1;
        for (const auto& [prime, exponent] : factorize(n)) {
            (void)prime; result *= static_cast<std::uint64_t>(exponent + 1);
        }
        return result;
    }
    std::uint64_t totient(std::uint64_t n) {
        auto result = n;
        for (const auto& [prime, exponent] : factorize(n)) {
            (void)exponent; result = result / prime * (prime - 1);
        }
        return result;
    }
    int mobius(std::uint64_t n) {
        int result = 1;
        for (const auto& [prime, exponent] : factorize(n)) {
            (void)prime;
            if (exponent > 1) return 0;
            result = -result;
        }
        return result;
    }
};
#endif

#ifndef SIRKIT_FACTORIZATION64_FUNCTIONS_DEFINED
#define SIRKIT_FACTORIZATION64_FUNCTIONS_DEFINED
inline std::vector<std::uint64_t> PrimeFactors64(std::uint64_t n) { return PollardRho64{}.prime_factors(n); }
inline std::vector<std::pair<std::uint64_t, int>> Factorize64(std::uint64_t n) { return PollardRho64{}.factorize(n); }
inline std::vector<std::uint64_t> Divisors64(std::uint64_t n, bool sorted = true) { return PollardRho64{}.divisors(n, sorted); }
inline std::uint64_t DivisorCount64(std::uint64_t n) { return PollardRho64{}.divisor_count(n); }
inline std::uint64_t EulerPhi64(std::uint64_t n) { return PollardRho64{}.totient(n); }
inline int Mobius64(std::uint64_t n) { return PollardRho64{}.mobius(n); }
#endif
