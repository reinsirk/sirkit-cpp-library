// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace
{
class Residue
{
public:
    static inline std::uint32_t modulus = 998244353;

    Residue() = default;
    explicit Residue(std::uint64_t value) : value_(static_cast<std::uint32_t>(value % modulus))
    {
    }

    std::uint32_t value() const
    {
        return value_;
    }

    Residue &operator+=(Residue other)
    {
        value_ += other.value_;
        if (value_ >= modulus)
        {
            value_ -= modulus;
        }
        return *this;
    }

    friend Residue operator-(Residue a, Residue b)
    {
        if (a.value_ < b.value_)
        {
            a.value_ += modulus;
        }
        a.value_ -= b.value_;
        return a;
    }

    friend Residue operator*(Residue a, Residue b)
    {
        return Residue(std::uint64_t{a.value_} * b.value_);
    }

    friend Residue operator/(Residue a, Residue b)
    {
        for (std::uint32_t exponent = modulus - 2; exponent != 0; exponent /= 2)
        {
            if (exponent % 2 == 1)
            {
                a = a * b;
            }
            b = b * b;
        }
        return a;
    }

    friend bool operator==(Residue, Residue) = default;

private:
    std::uint32_t value_{};
};

bool is_prime(std::uint32_t value)
{
    for (std::uint32_t divisor = 3; divisor * divisor <= value; divisor += 2)
    {
        if (value % divisor == 0)
        {
            return false;
        }
    }
    return true;
}
} // namespace

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<int> parents(n, -1);
    for (int vertex = 1; vertex < n; ++vertex)
    {
        verify_in >> parents[vertex];
        --parents[vertex];
    }
    std::vector<std::pair<int, int>> backward(m);
    for (auto &[from, to] : backward)
    {
        verify_in >> from >> to;
        --from;
        --to;
    }
    const sirkit::TreeBackedgeAbsorption walk(parents, backward);
    const auto try_prime = [&](std::uint32_t prime)
    {
        Residue::modulus = prime;
        const auto answer = walk.expected_steps<Residue>();
        if (answer)
        {
            verify_out << answer->value() << ' ' << prime << '\n';
        }
        return answer.has_value();
    };
    if (try_prime(998244353) || try_prime(1000000007))
    {
        return 0;
    }
    std::mt19937 random(std::random_device{}());
    std::uniform_int_distribution<std::uint32_t> indices(0, 500877823);
    while (true)
    {
        const std::uint32_t candidate = 998244353 + 2 * indices(random);
        if (is_prime(candidate) && try_prime(candidate))
        {
            return 0;
        }
    }
}
