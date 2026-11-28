// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Pair_Xor_Order.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

template <class T>
void Check(std::vector<T> values, std::mt19937_64 &random)
{
    std::vector<T> reference;
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        for (std::size_t j = i + 1; j < values.size(); ++j)
        {
            reference.push_back(values[i] ^ values[j]);
        }
    }
    std::sort(reference.begin(), reference.end());
    const sirkit::PairXorOrder order(std::move(values));
    assert(order.pair_count() == reference.size());
    assert(order.count_less(0) == 0);
    for (std::size_t i = 0; i < reference.size(); ++i)
    {
        assert(order.kth(i) == reference[i]);
    }
    for (int i = 0; i < 31; ++i)
    {
        const T bound = static_cast<T>(random());
        const auto true_count = std::lower_bound(reference.begin(), reference.end(), bound) - reference.begin();
        assert(order.count_less(bound) == static_cast<std::uint64_t>(true_count));
    }
}

int main()
{
    using Bits = std::uint64_t;
    std::mt19937_64 random(394674);
    const sirkit::PairXorOrder<Bits> empty({});
    assert(empty.count_less(2) == 0 && empty.pair_count() == 0);
    Check(std::vector<Bits>{1}, random);
    Check(std::vector<Bits>{0, 0, 0, 0}, random);
    Check(std::vector<Bits>{0, UINT64_MAX, UINT64_MAX, UINT64_MAX - 1}, random);
    for (int i = 0; i < 220; ++i)
    {
        std::vector<std::uint8_t> small(random() % 31);
        std::vector<Bits> large(random() % 21);
        for (auto &value : small)
        {
            value = random() % (i % 2 ? 64 : 251);
        }
        for (auto &value : large)
        {
            value = random();
        }
        Check(std::move(small), random);
        Check(std::move(large), random);
    }

    const std::vector<unsigned> repeated(100000, 72);
    const sirkit::PairXorOrder order(repeated);
    assert(order.pair_count() == UINT64_C(4999950000));
    assert(order.kth(order.pair_count() - 1) == 0 && order.count_less(1) == order.pair_count());

#ifndef SKIP_RUNTIME_CHECKS
    bool invalid_rank = false;
    try
    {
        (void)order.kth(order.pair_count());
    }
    catch (const std::out_of_range &)
    {
        invalid_rank = true;
    }
    assert(invalid_rank);
#endif
    if constexpr (sizeof(std::size_t) > sizeof(std::uint32_t))
    {
        bool bad_size = false;
        try
        {
            sirkit::detail::CheckPairXorSize(std::size_t{1} << 32);
        }
        catch (const std::length_error &)
        {
            bad_size = true;
        }
        assert(bad_size);
    }
}
