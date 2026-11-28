// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Polynomial_Factorization.cpp
// 参考（有限体上の因数分解）: https://shoup.net/ntb/ntb-v2.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_factorization.md
//
// API一覧
// [PolynomialFactor]
// coefficients, multiplicity: モニック既約因子の係数（低次順）と重複度。
// [PolynomialFactorization]
// leading_coefficient, factors: 元の最高次係数と、相異なる既約因子の列。
// factor_polynomial(coefficients,prime,seed): 素数primeを法として非零多項式を因数分解する。
// primeはuint32_t。係数は剰余へ正規化する。零多項式はdomain_error、合成数の法はinvalid_argument。
// Nは次数+1、Lは入力の係数数。期待O(L+N³ log(prime)+sqrt(prime))時間、O(L)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_POLYNOMIAL_FACTORIZATION_DEFINED
#define SIRKIT_POLYNOMIAL_FACTORIZATION_DEFINED
struct PolynomialFactor
{
    std::vector<std::uint32_t> coefficients;
    std::size_t multiplicity;
};

struct PolynomialFactorization
{
    std::uint32_t leading_coefficient;
    std::vector<PolynomialFactor> factors;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
class PrimePolynomialFactorizer
{
    using Residue = std::uint32_t;
    using Polynomial = std::vector<Residue>;

    const Residue prime_;
    std::mt19937_64 engine_;
    std::vector<PolynomialFactor> factors_;

    static Residue CheckedPrime(Residue prime)
    {
        if (prime < 2)
        {
            throw std::invalid_argument("factor_polynomial: prime modulus required");
        }
        for (Residue divisor = 2; divisor <= prime / divisor; ++divisor)
        {
            if (prime % divisor == 0)
            {
                throw std::invalid_argument("factor_polynomial: prime modulus required");
            }
        }
        return prime;
    }

    static void Trim(Polynomial &value)
    {
        while (!value.empty() && value.back() == 0)
        {
            value.pop_back();
        }
    }

    [[nodiscard]] Residue add(Residue a, Residue b) const noexcept
    {
        const auto threshold = prime_ - b;
        return a >= threshold ? a - threshold : a + b;
    }

    [[nodiscard]] Residue subtract(Residue a, Residue b) const noexcept
    {
        return a >= b ? a - b : a + (prime_ - b);
    }

    [[nodiscard]] Residue multiply(Residue a, Residue b) const noexcept
    {
        return static_cast<Residue>(std::uint64_t{a} * b % prime_);
    }

    [[nodiscard]] Residue inverse(Residue a) const noexcept
    {
        Residue result = 1;
        for (auto exponent = prime_ - 2; exponent > 0; exponent >>= 1)
        {
            if ((exponent & 1U) != 0)
            {
                result = multiply(result, a);
            }
            a = multiply(a, a);
        }
        return result;
    }

    void make_monic(Polynomial &value) const
    {
        if (value.back() != 1)
        {
            const auto scale = inverse(value.back());
            for (auto &coefficient : value)
            {
                coefficient = multiply(coefficient, scale);
            }
        }
    }

    void reduce(Polynomial &value, const Polynomial &modulus, Polynomial *quotient = nullptr) const
    {
        if (value.size() < modulus.size())
        {
            return;
        }
        const auto degree = modulus.size() - 1;
        const auto steps = value.size() - degree;
        const auto reciprocal = inverse(modulus.back());
        if (quotient != nullptr)
        {
            quotient->resize(steps);
        }
        for (auto i = steps; i != 0; --i)
        {
            const auto offset = i - 1;
            const auto coefficient = multiply(value[degree + offset], reciprocal);
            if (quotient != nullptr)
            {
                (*quotient)[offset] = coefficient;
            }
            if (coefficient != 0)
            {
                for (std::size_t j = 0; j < degree; ++j)
                {
                    value[offset + j] = subtract(value[offset + j], multiply(coefficient, modulus[j]));
                }
            }
        }
        value.resize(degree);
        Trim(value);
    }

    [[nodiscard]] Polynomial divide(Polynomial value, const Polynomial &divisor) const
    {
        Polynomial quotient;
        reduce(value, divisor, &quotient);
        return quotient;
    }

    [[nodiscard]] Polynomial gcd(Polynomial a, Polynomial b) const
    {
        while (!b.empty())
        {
            reduce(a, b);
            a.swap(b);
        }
        make_monic(a);
        return a;
    }

    [[nodiscard]] Polynomial product_mod(const Polynomial &a, const Polynomial &b, const Polynomial &modulus) const
    {
        if (a.empty() || b.empty())
        {
            return {};
        }
        Polynomial result(a.size() + b.size() - 1);
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            for (std::size_t j = 0; j < b.size(); ++j)
            {
                result[i + j] = add(result[i + j], multiply(a[i], b[j]));
            }
        }
        reduce(result, modulus);
        return result;
    }

    [[nodiscard]] Polynomial power_mod(Polynomial base, Residue exponent, const Polynomial &modulus) const
    {
        Polynomial result{1};
        while (exponent != 0)
        {
            if ((exponent & 1U) != 0)
            {
                result = product_mod(result, base, modulus);
            }
            exponent >>= 1;
            if (exponent != 0)
            {
                base = product_mod(base, base, modulus);
            }
        }
        return result;
    }

    [[nodiscard]] Polynomial split_equal_degree(const Polynomial &value, std::size_t degree)
    {
        std::uniform_int_distribution<Residue> random(0, prime_ - 1);
        while (true)
        {
            Polynomial sample(value.size() - 1);
            for (auto &coefficient : sample)
            {
                coefficient = random(engine_);
            }
            Trim(sample);
            Polynomial separator;
            if (prime_ == 2)
            {
                // 標数2ではトレースの値0と1で分ける。
                separator = sample;
                for (std::size_t i = 1; i < degree; ++i)
                {
                    sample = product_mod(sample, sample, value);
                    separator.resize(std::max(separator.size(), sample.size()));
                    for (std::size_t j = 0; j < sample.size(); ++j)
                    {
                        separator[j] ^= sample[j];
                    }
                }
                Trim(separator);
            }
            else
            {
                // (p^d-1)/2乗を、(p-1)/2乗とFrobenius写像で計算する。
                sample = power_mod(std::move(sample), (prime_ - 1) / 2, value);
                separator = sample;
                for (std::size_t i = 1; i < degree; ++i)
                {
                    sample = power_mod(std::move(sample), prime_, value);
                    separator = product_mod(separator, sample, value);
                }
                separator.resize(std::max(std::size_t{1}, separator.size()));
                separator[0] = subtract(separator[0], 1);
                Trim(separator);
            }
            auto factor = gcd(value, std::move(separator));
            if (factor.size() > 1 && factor.size() < value.size())
            {
                return factor;
            }
        }
    }

    void factor_same_degree(Polynomial value, std::size_t degree, std::size_t multiplicity)
    {
        std::vector<Polynomial> pending;
        pending.push_back(std::move(value));
        while (!pending.empty())
        {
            auto next = std::move(pending.back());
            pending.pop_back();
            if (next.size() == degree + 1)
            {
                factors_.push_back({std::move(next), multiplicity});
            }
            else
            {
                auto factor = split_equal_degree(next, degree);
                pending.push_back(divide(std::move(next), factor));
                pending.push_back(std::move(factor));
            }
        }
    }

    void factor_square_free(Polynomial value, std::size_t multiplicity)
    {
        Polynomial frobenius{0, 1};
        // x^(p^d)-xの既約因子は次数がdの約数。小さい次数から除く。
        for (std::size_t degree = 1; degree <= (value.size() - 1) / 2; ++degree)
        {
            frobenius = power_mod(std::move(frobenius), prime_, value);
            auto difference = frobenius;
            difference.resize(std::max(std::size_t{2}, difference.size()));
            difference[1] = subtract(difference[1], 1);
            Trim(difference);
            auto selected = gcd(value, std::move(difference));
            if (selected.size() > 1)
            {
                value = divide(std::move(value), selected);
                factor_same_degree(std::move(selected), degree, multiplicity);
                reduce(frobenius, value);
            }
        }
        if (value.size() > 1)
        {
            factors_.push_back({std::move(value), multiplicity});
        }
    }

    void remove_multiplicities(Polynomial value, std::size_t multiplicity_scale)
    {
        Polynomial derivative(value.size() - 1);
        for (std::size_t i = 1; i < value.size(); ++i)
        {
            derivative[i - 1] = multiply(value[i], static_cast<Residue>(i % prime_));
        }
        Trim(derivative);
        auto repeated = gcd(value, std::move(derivative));
        auto remaining = divide(std::move(value), repeated);
        for (std::size_t multiplicity = 1; remaining.size() > 1; ++multiplicity)
        {
            auto overlap = gcd(remaining, repeated);
            auto exact = divide(std::move(remaining), overlap);
            if (exact.size() > 1)
            {
                factor_square_free(std::move(exact), multiplicity * multiplicity_scale);
            }
            repeated = divide(std::move(repeated), overlap);
            remaining = std::move(overlap);
        }
        if (repeated.size() > 1)
        {
            // 微分が0の残りはp乗。指数をpで割った多項式に再帰する。
            Polynomial root((repeated.size() - 1) / prime_ + 1);
            for (std::size_t i = 0; i < root.size(); ++i)
            {
                root[i] = repeated[i * prime_];
            }
            remove_multiplicities(std::move(root), multiplicity_scale * prime_);
        }
    }

public:
    PrimePolynomialFactorizer(Residue prime, std::uint64_t seed) : prime_(CheckedPrime(prime)), engine_(seed)
    {
    }

    [[nodiscard]] PolynomialFactorization run(std::span<const Residue> coefficients)
    {
        Polynomial value(coefficients.begin(), coefficients.end());
        for (auto &coefficient : value)
        {
            coefficient %= prime_;
        }
        Trim(value);
        if (value.empty())
        {
            throw std::domain_error("factor_polynomial: zero polynomial");
        }
        const auto leading = value.back();
        make_monic(value);
        if (value.size() > 1)
        {
            remove_multiplicities(std::move(value), 1);
        }
        std::ranges::sort(factors_, {}, &PolynomialFactor::coefficients);
        return {leading, std::move(factors_)};
    }
};
}

inline PolynomialFactorization factor_polynomial(std::span<const std::uint32_t> coefficients, std::uint32_t prime,
                                                 std::uint64_t seed = std::random_device{}())
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::PrimePolynomialFactorizer(prime, seed).run(coefficients);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
