#include <optional>

#ifndef SIRKIT_PRIME_MULTIPLICATIVE_GROUP_DEFINED
#define SIRKIT_PRIME_MULTIPLICATIVE_GROUP_DEFINED
class PrimeMultiplicativeGroup
{
public:
    explicit PrimeMultiplicativeGroup(std::uint64_t prime) : modulus_(prime), factors_(factor_group_size(prime))
    {
    }

    [[nodiscard]] std::uint64_t modulus() const noexcept
    {
        return modulus_;
    }

    [[nodiscard]] std::optional<std::uint64_t> order(std::uint64_t a) const noexcept
    {
        a %= modulus_;
        if (a == 0)
        {
            return std::nullopt;
        }
        auto result = modulus_ - 1;
        if (modulus_ == 2)
        {
            return result;
        }
        const detail::FactorizationArithmetic arithmetic(modulus_);
        const auto encoded = arithmetic.encode(a);
        for (const auto &[factor, exponent] : factors_)
        {
            for (int degree = 0; degree < exponent; ++degree)
            {
                if (arithmetic.power(encoded, result / factor) != arithmetic.one)
                {
                    break;
                }
                result /= factor;
            }
        }
        return result;
    }

    [[nodiscard]] bool is_generator(std::uint64_t a) const noexcept
    {
        a %= modulus_;
        if (a == 0)
        {
            return false;
        }
        if (modulus_ == 2)
        {
            return true;
        }
        return generates(detail::FactorizationArithmetic(modulus_), a);
    }

    [[nodiscard]] std::uint64_t primitive_root() const noexcept
    {
        if (modulus_ == 2)
        {
            return 1;
        }
        const detail::FactorizationArithmetic arithmetic(modulus_);
        std::uint64_t candidate = 2;
        while (!generates(arithmetic, candidate))
        {
            ++candidate;
        }
        return candidate;
    }

private:
    std::uint64_t modulus_;
    std::vector<std::pair<std::uint64_t, int>> factors_;

    static std::vector<std::pair<std::uint64_t, int>> factor_group_size(std::uint64_t prime)
    {
        if (!PollardRho64::is_prime(prime))
        {
            throw std::domain_error("PrimeMultiplicativeGroup: modulus must be prime");
        }
        return Factorize64(prime - 1);
    }

    [[nodiscard]] bool generates(const detail::FactorizationArithmetic &arithmetic, std::uint64_t a) const noexcept
    {
        const auto encoded = arithmetic.encode(a);
        for (const auto &[factor, exponent] : factors_)
        {
            if (arithmetic.power(encoded, (modulus_ - 1) / factor) == arithmetic.one)
            {
                return false;
            }
        }
        return true;
    }
};

[[nodiscard]] inline std::uint64_t primitive_root(std::uint64_t prime)
{
    return PrimeMultiplicativeGroup(prime).primitive_root();
}
#endif
