// SPDX-License-Identifier: CC0-1.0
// 参考（有限体の根抽出）: https://www.cs.cmu.edu/~glmiller/Publications/AMM77.pdf
// 参考（素数冪の離散対数）: https://www-ee.stanford.edu/~hellman/publications/28.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_kth_root.md
//
// API一覧
// modular_kth_root(k,y,p): x^k≡y (mod p) となるx ∈ [0,p) を1つ返す。解なしはnullopt。
// kは非負のuint64_t、yはuint64_tでpにより縮約。pはuint32_tの素数。0^0=1。
// d=gcd(k,p-1) の素因数分解と、必要な素数位数部分群だけの離散対数を用いる。
// 離散対数の表はℓ²|(p-1)を満たすℓについてO(√ℓ)領域。時間の内訳はドキュメント。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MODULAR_KTH_ROOT_DEFINED
#define SIRKIT_MODULAR_KTH_ROOT_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
class KthRootPrimeField32
{
public:
    explicit KthRootPrimeField32(std::uint32_t modulus) : modulus_(modulus)
    {
    }

    std::uint32_t modulus() const
    {
        return modulus_;
    }

    std::uint32_t multiply(std::uint32_t a, std::uint32_t b) const
    {
        return static_cast<std::uint32_t>((std::uint64_t{a} * b) % modulus_);
    }

    std::uint32_t power(std::uint32_t a, std::uint32_t exponent) const
    {
        std::uint32_t result = 1;
        while (exponent != 0)
        {
            if (exponent & 1)
            {
                result = multiply(result, a);
            }
            a = multiply(a, a);
            exponent >>= 1;
        }
        return result;
    }

private:
    std::uint32_t modulus_;
};

inline std::uint32_t KthRootCoprimeInverse(std::uint64_t a, std::uint32_t modulus)
{
    std::int64_t previous = modulus;
    std::int64_t current = static_cast<std::int64_t>(a % modulus);
    std::int64_t previous_coeff = 0, coeff = 1;
    while (current != 0)
    {
        const auto quotient = previous / current;
        const auto remainder = previous % current;
        const auto next_coeff = previous_coeff - quotient * coeff;
        previous = current;
        current = remainder;
        previous_coeff = coeff;
        coeff = next_coeff;
    }
    return static_cast<std::uint32_t>(previous_coeff < 0 ? previous_coeff + modulus : previous_coeff);
}

inline const std::vector<std::uint32_t> &KthRootTrialPrimes()
{
    static const auto primes = []
    {
        // 非自明な解があるとき d は p-1 の真の約数。d ≤ (p-1)/2 < 2³¹。
        constexpr std::uint32_t kLimit = 46'341;
        std::array<bool, kLimit> composite{};
        std::vector<std::uint32_t> result;
        for (std::uint32_t i = 2; i < kLimit; ++i)
        {
            if (composite[i])
            {
                continue;
            }
            result.push_back(i);
            if (i * i < kLimit)
            {
                for (auto j = i * i; j < kLimit; j += i)
                {
                    composite[j] = true;
                }
            }
        }
        return result;
    }();
    return primes;
}

class KthRootPrimeOrderLog
{
public:
    KthRootPrimeOrderLog(const KthRootPrimeField32 &field, std::uint32_t generator, std::uint32_t order)
        : field_(field), order_(order)
    {
        if (order == 2)
        {
            return;
        }
        // この表が必要なℓは ℓ² < 2³² を満たすので、ceil(sqrt(ℓ)) ≤ 256。
        width_ = static_cast<std::uint32_t>(std::sqrt(order - 1)) + 1;
        std::uint32_t residue = 1;
        for (std::uint32_t i = 0; i < width_; ++i)
        {
            entries_[i] = (std::uint64_t{residue} << 32) | i;
            residue = field_.multiply(residue, generator);
        }
        std::sort(entries_.begin(), entries_.begin() + width_);
        inverse_stride_ = field_.power(generator, order - width_);
    }

    std::uint32_t operator()(std::uint32_t target) const
    {
        if (target == 1)
        {
            return 0;
        }
        if (order_ == 2)
        {
            return 1;
        }

        const auto last = entries_.begin() + width_;
        for (std::uint32_t giant = 0;; giant += width_)
        {
            const auto key = std::uint64_t{target} << 32;
            const auto match = std::lower_bound(entries_.begin(), last, key);
            if (match != last && (*match >> 32) == target)
            {
                return giant + static_cast<std::uint32_t>(*match);
            }
            target = field_.multiply(target, inverse_stride_);
        }
    }

private:
    const KthRootPrimeField32 &field_;
    std::uint32_t order_, width_ = 0, inverse_stride_ = 1;
    std::array<std::uint64_t, 256> entries_;
};

inline std::uint32_t LiftKthRootPrimePower(const KthRootPrimeField32 &field, std::uint32_t y,
                                           std::uint32_t factor, std::uint32_t degree)
{
    const std::uint32_t group_size = field.modulus() - 1;
    auto coprime = group_size / degree;
    while (coprime % factor == 0)
    {
        coprime /= factor;
    }
    const auto baseline_power = KthRootCoprimeInverse(degree, coprime);
    const auto baseline = field.power(y, baseline_power);
    // degree*baseline_power≡1 (mod coprime)。残差はfactorの冪位数の部分群にある。
    const auto past = static_cast<std::uint32_t>((std::uint64_t{degree} * baseline_power) % group_size);
    const auto residual_power = (std::uint64_t{group_size} + 1 - past) % group_size;
    auto residual = field.power(y, static_cast<std::uint32_t>(residual_power));
    if (residual == 1)
    {
        return baseline;
    }

    std::uint32_t base = 2;
    while (field.power(base, group_size / factor) == 1)
    {
        ++base;
    }
    const auto generator = field.power(base, coprime);
    const auto reduced_generator = field.power(generator, degree);
    const auto order = group_size / coprime / degree;
    const KthRootPrimeOrderLog digit_log(field, field.power(reduced_generator, order / factor), factor);
    auto inverse_power = field.power(reduced_generator, order - 1);
    std::uint32_t log = 0;
    for (std::uint32_t place = 1, remaining = order; residual != 1; place *= factor, remaining /= factor)
    {
        const auto digit = digit_log(field.power(residual, remaining / factor));
        log += place * digit;
        residual = field.multiply(residual, field.power(inverse_power, digit));
        inverse_power = field.power(inverse_power, factor);
    }
    return field.multiply(baseline, field.power(generator, log));
}
}

inline std::optional<std::uint32_t> modular_kth_root(std::uint64_t k, std::uint64_t y, std::uint32_t p)
{
    if (p < 2)
    {
        throw std::invalid_argument("modular_kth_root: prime required");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (p != 2 && (p & 1) == 0)
    {
        throw std::invalid_argument("modular_kth_root: prime required");
    }
#endif
    const auto target = static_cast<std::uint32_t>(y % p);
    if (k == 0)
    {
        if (target == 1)
        {
            return 0;
        }
        return std::nullopt;
    }
    if (target <= 1)
    {
        return target;
    }

    const /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KthRootPrimeField32 field(p);
    const auto group_size = p - 1;
    auto divisor = static_cast<std::uint32_t>(std::gcd(k, std::uint64_t{group_size}));
    const auto reduced_order = group_size / divisor;
    if (divisor == 1)
    {
        return field.power(target, /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KthRootCoprimeInverse(k, reduced_order));
    }
    if (field.power(target, reduced_order) != 1)
    {
        return std::nullopt;
    }

    auto root = field.power(target, /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KthRootCoprimeInverse(k / divisor, reduced_order));
    for (const auto factor : /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KthRootTrialPrimes())
    {
        if (factor * factor > divisor)
        {
            break;
        }
        if (divisor % factor != 0)
        {
            continue;
        }
        std::uint32_t degree = 1;
        do
        {
            divisor /= factor;
            degree *= factor;
        } while (divisor % factor == 0);
        root = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::LiftKthRootPrimePower(field, root, factor, degree);
    }
    if (divisor != 1)
    {
        root = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::LiftKthRootPrimePower(field, root, divisor, divisor);
    }
    return root;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
