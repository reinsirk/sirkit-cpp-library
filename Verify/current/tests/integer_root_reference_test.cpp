// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_INTEGER_ROOT_TEST_SOURCE
#define SIRKIT_INTEGER_ROOT_TEST_SOURCE "../../../Math/Integer_Root.cpp"
#endif
#include SIRKIT_INTEGER_ROOT_TEST_SOURCE
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

namespace
{
bool RepeatedProductAtMost(std::uint64_t value, unsigned exponent, std::uint64_t limit)
{
    if (value == 0)
    {
        return true;
    }
    while (exponent-- != 0)
    {
        limit /= value;
    }
    return limit != 0;
}

void Check(std::uint64_t n, unsigned k)
{
    const auto value = sirkit::integer_kth_root(n, k);
    assert(RepeatedProductAtMost(value, k, n));
    if (value != std::numeric_limits<std::uint64_t>::max())
    {
        assert(!RepeatedProductAtMost(value + 1, k, n));
    }
}
} // namespace

int main()
{
    for (std::uint64_t n = 0; n <= 1024; ++n)
    {
        for (unsigned k = 1; k <= 66; ++k)
        {
            Check(n, k);
        }
    }

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    for (unsigned k = 1; k <= 66; ++k)
    {
        Check(kMax, k);
        Check(kMax - 1, k);
    }
    assert(sirkit::integer_kth_root(kMax, 1000000000) == 1);
    // わざと外した初期候補から、整数の二分探索に戻れることを確認する。
    assert(sirkit::detail::RefineIntegerRoot(4000, 2, 1, 64) == 63);
    assert(sirkit::detail::RefineIntegerRoot(3300, 2, 62, 64) == 57);
    assert(sirkit::detail::RefineIntegerRoot(200, 2, 5, 16) == 14);
    std::mt19937_64 random(541663);
    for (int i = 0; i < 14000; ++i)
    {
        const auto value = random();
        Check(value, 2);
        Check(value, 3);
        Check(value, static_cast<unsigned>(random() % 66 + 1));
        const auto root = (random() >> 32) + 1;
        if (root <= std::numeric_limits<std::uint32_t>::max())
        {
            const auto perfect = root * root;
            Check(perfect - 1, 2);
            Check(perfect, 2);
            Check(perfect + 1, 2);
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    bool rejected = false;
    try
    {
        sirkit::integer_kth_root(1, 0);
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
#endif
}
