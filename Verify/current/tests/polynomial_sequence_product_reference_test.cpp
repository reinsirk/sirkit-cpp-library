// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/NTT.cpp"
#include "../../../Math/Polynomial_Sequence_Product.cpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    using Polynomial = std::vector<std::uint32_t>;
    constexpr std::uint32_t kMod = 998244353;
    assert((sirkit::polynomial_sequence_product({}) == Polynomial{1}));
    assert((sirkit::polynomial_sequence_product({{5}, {7}, {kMod + 2, 0, kMod}}) == Polynomial{70}));
    assert(sirkit::polynomial_sequence_product({{1, 2}, {}, {1, 1}}).empty());
    assert(sirkit::polynomial_sequence_product({{kMod, 0}}).empty());
    assert((sirkit::polynomial_sequence_product({{1, kMod + 1}}) == Polynomial{1, 1}));
    std::mt19937 random(8831);
    for (unsigned trial = 0; trial < 460; ++trial)
    {
        std::vector<Polynomial> values(random() % 17);
        Polynomial expected{1};
        for (auto &factor : values)
        {
            factor.resize(random() % 22 + 1);
            Polynomial next(expected.size() + factor.size() - 1);
            for (auto &x : factor)
            {
                x = random();
            }
            for (std::size_t i = 0; i < expected.size(); ++i)
            {
                for (std::size_t j = 0; j < factor.size(); ++j)
                {
                    next[i + j] = (next[i + j] + std::uint64_t{expected[i]} * (factor[j] % kMod)) % kMod;
                }
            }
            while (!next.empty() && next.back() == 0)
            {
                next.pop_back();
            }
            if (!expected.empty())
            {
                expected = std::move(next);
            }
        }
        assert(sirkit::polynomial_sequence_product(values) == expected);
    }
    constexpr unsigned kSize = 4096;
    const std::vector<Polynomial> equal_factors(kSize, Polynomial{1, 1});
    const auto power = sirkit::polynomial_sequence_product(equal_factors);
    assert(power.size() == kSize + 1 && power.front() == 1 && power.back() == 1);
    for (unsigned i = 0; i < kSize; ++i)
    {
        assert(std::uint64_t{power[i]} * (kSize - i) % kMod ==
               std::uint64_t{power[i + 1]} * (i + 1) % kMod);
    }
    try
    {
        std::vector<Polynomial> too_long{Polynomial(sirkit::NTT<>::max_length() + 1, 1)};
        static_cast<void>(sirkit::polynomial_sequence_product(std::move(too_long)));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
