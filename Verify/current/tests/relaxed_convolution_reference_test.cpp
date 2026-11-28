// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Relaxed_Convolution.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    constexpr std::uint32_t kMod = 998244353;
    sirkit::RelaxedConvolution convolution;
    assert(convolution.size() == 0);
    std::mt19937 random(5033);
    for (const std::size_t n : {1, 7, 31, 32, 63, 128, 137, 1089, 2049})
    {
        std::vector<std::uint32_t> left(n), right(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            const auto a = static_cast<std::uint32_t>(random());
            const auto b = static_cast<std::uint32_t>(random());
            left[i] = a % kMod;
            right[i] = b % kMod;
            std::uint32_t expected{};
            for (std::size_t j = 0; j <= i; ++j)
            {
                expected = static_cast<std::uint32_t>((expected + std::uint64_t{left[j]} * right[i - j]) % kMod);
            }
            assert(convolution.push(a, b) == expected);
            assert(convolution.size() == i + 1);
        }
        convolution.clear();
        assert(convolution.size() == 0);
    }

    // 小さい法で最大変換長と、それを越えても状態が保たれることを確かめる。
    sirkit::RelaxedConvolution<17, 3> small;
    assert(small.max_size() == 16);
    for (std::uint32_t i = 0; i < small.max_size(); ++i)
    {
        assert(small.push(2, 3) == (i + 1) * 6 % 17);
    }
    try
    {
        static_cast<void>(small.push(7, 8));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    assert(small.size() == small.max_size());
    small.clear();
    assert(small.push(7, 8) == 5);
}
