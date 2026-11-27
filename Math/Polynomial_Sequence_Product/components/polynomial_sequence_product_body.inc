#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_POLYNOMIAL_SEQUENCE_PRODUCT_DEFINED
#define SIRKIT_POLYNOMIAL_SEQUENCE_PRODUCT_DEFINED

[[nodiscard]] inline std::vector<std::uint32_t> polynomial_sequence_product(
    std::vector<std::vector<std::uint32_t>> factors)
{
    constexpr auto kMod = NTT<>::modulus();
    std::uint32_t scalar = 1;
    std::uint64_t degree = 0;
    std::vector<std::size_t> heap;
    for (std::size_t i = 0; i < factors.size(); ++i)
    {
        auto &factor = factors[i];
        for (auto &coefficient : factor)
        {
            coefficient %= kMod;
        }
        while (!factor.empty() && factor.back() == 0)
        {
            factor.pop_back();
        }
        if (factor.empty())
        {
            return {};
        }
        if (factor.size() == 1)
        {
            scalar = static_cast<std::uint32_t>(std::uint64_t{scalar} * factor.front() % kMod);
            continue;
        }
        degree += factor.size() - 1;
        heap.push_back(i);
    }
    if (heap.empty())
    {
        return {scalar};
    }
    if (degree >= NTT<>::max_length())
    {
        throw std::length_error("polynomial_sequence_product: total degree exceeds transform budget");
    }

    const auto longer = [&](std::size_t left, std::size_t right)
    {
        return factors[left].size() > factors[right].size();
    };
    std::make_heap(heap.begin(), heap.end(), longer);
    const auto pop_shortest = [&]
    {
        std::pop_heap(heap.begin(), heap.end(), longer);
        const auto index = heap.back();
        heap.pop_back();
        return index;
    };
    NTT<> ntt;
    while (heap.size() > 1)
    {
        const auto first = pop_shortest();
        const auto second = pop_shortest();
        factors[first] = ntt.convolution_normalized(std::move(factors[first]), std::move(factors[second]));
        heap.push_back(first);
        std::push_heap(heap.begin(), heap.end(), longer);
    }
    auto result = std::move(factors[heap.front()]);
    if (scalar != 1)
    {
        for (auto &coefficient : result)
        {
            coefficient = static_cast<std::uint32_t>(std::uint64_t{coefficient} * scalar % kMod);
        }
    }
    return result;
}
#endif
