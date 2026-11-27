#include <cstdlib>

#ifndef SIRKIT_TWO_SQUARE_SUM_DEFINED
#define SIRKIT_TWO_SQUARE_SUM_DEFINED
namespace detail
{
struct TwoSquareGaussian
{
    std::int64_t real;
    std::int64_t imag;

    [[nodiscard]] TwoSquareGaussian operator*(const TwoSquareGaussian &other) const
    {
        return {real * other.real - imag * other.imag, real * other.imag + imag * other.real};
    }
};

inline TwoSquareGaussian factor_two_square_prime(std::uint64_t prime)
{
    const FactorizationArithmetic arithmetic(prime);
    std::uint64_t candidate = 2;
    std::uint64_t root;
    do
    {
        root = arithmetic.power(arithmetic.encode(candidate), (prime - 1) / 4);
        ++candidate;
    } while (arithmetic.multiply(root, root) != prime - arithmetic.one);
    root = arithmetic.multiply(root, 1);
    std::uint64_t a = prime;
    std::uint64_t b = std::min(root, prime - root);
    while (b > prime / b)
    {
        const auto remainder = a % b;
        a = b;
        b = remainder;
    }
    const auto other = factor_square_root(prime - b * b);
    return {static_cast<std::int64_t>(b), static_cast<std::int64_t>(other)};
}
} // namespace detail

[[nodiscard]] inline std::vector<std::pair<std::uint64_t, std::uint64_t>> two_square_sum(std::uint64_t n)
{
    if (n == 0)
    {
        return {{0, 0}};
    }
    const auto factors = Factorize64(n);
    detail::TwoSquareGaussian common{1, 0};
    for (const auto &[p, e] : factors)
    {
        if (p == 2)
        {
            for (int i = 0; i < e; ++i)
            {
                common = common * detail::TwoSquareGaussian{1, 1};
            }
        }
        else if (p % 4 == 3)
        {
            if (e % 2 != 0)
            {
                return {};
            }
            for (int i = 0; i < e / 2; ++i)
            {
                common = common * detail::TwoSquareGaussian{static_cast<std::int64_t>(p), 0};
            }
        }
    }

    std::vector<detail::TwoSquareGaussian> choices{common};
    for (const auto &[p, e] : factors)
    {
        if (p % 4 != 1)
        {
            continue;
        }
        const auto gaussian = detail::factor_two_square_prime(p);
        std::vector<detail::TwoSquareGaussian> powers{{1, 0}};
        powers.reserve(static_cast<std::size_t>(e) + 1);
        for (int i = 0; i < e; ++i)
        {
            powers.push_back(powers.back() * gaussian);
        }
        std::vector<detail::TwoSquareGaussian> next;
        next.reserve(choices.size() * powers.size());
        for (int i = 0; i <= e; ++i)
        {
            const auto positive = powers[i];
            const auto negative = powers[e - i];
            const auto factor = positive * detail::TwoSquareGaussian{negative.real, -negative.imag};
            for (const auto &choice : choices)
            {
                next.push_back(choice * factor);
            }
        }
        choices.swap(next);
    }

    std::vector<std::pair<std::uint64_t, std::uint64_t>> result;
    result.reserve(2 * choices.size());
    for (const auto &[real, imag] : choices)
    {
        const auto x = static_cast<std::uint64_t>(std::abs(real));
        const auto y = static_cast<std::uint64_t>(std::abs(imag));
        result.emplace_back(x, y);
        result.emplace_back(y, x);
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}
#endif
