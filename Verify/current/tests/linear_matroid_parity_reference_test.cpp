// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Linear_Matroid_Parity.cpp"
#include "reference_modint.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

static constexpr int kPrime = 1'000'000'007;
using Mint = sirkit_verify::ReferenceModInt<kPrime>;
using Parity = sirkit::LinearMatroidParity<Mint>;
using Sparse = std::vector<std::pair<std::size_t, int>>;
using Pair = std::array<Sparse, 2>;

static bool independent(std::vector<std::vector<std::int64_t>> columns, std::size_t dimension)
{
    std::size_t rank = 0;
    for (std::size_t row = 0; row < dimension && rank < columns.size(); ++row)
    {
        auto pivot = rank;
        while (pivot < columns.size() && columns[pivot][row] == 0)
        {
            ++pivot;
        }
        if (pivot == columns.size())
        {
            continue;
        }
        std::swap(columns[pivot], columns[rank]);
        const auto scale = columns[rank][row];
        for (auto i = rank + 1; i < columns.size(); ++i)
        {
            const auto remove = columns[i][row];
            for (auto j = row; j < dimension; ++j)
            {
                columns[i][j] = (columns[i][j] * scale - columns[rank][j] * remove) % kPrime;
            }
        }
        ++rank;
    }
    return rank == columns.size();
}

static std::size_t brute(std::size_t dimension, const std::vector<Pair> &pairs)
{
    std::size_t best = 0;
    for (std::uint32_t mask = 1; mask < (std::uint32_t{1} << pairs.size()); ++mask)
    {
        const auto count = static_cast<std::size_t>(std::popcount(mask));
        if (count <= best || 2 * count > dimension)
        {
            continue;
        }
        std::vector<std::vector<std::int64_t>> columns;
        for (std::size_t i = 0; i < pairs.size(); ++i)
        {
            if ((mask >> i & 1u) != 0)
            {
                for (const auto &vector : pairs[i])
                {
                    auto &column = columns.emplace_back(dimension);
                    for (const auto &[index, value] : vector)
                    {
                        column[index] += value;
                    }
                }
            }
        }
        if (independent(std::move(columns), dimension))
        {
            best = count;
        }
    }
    return best;
}

static std::size_t sample(std::size_t dimension, const std::vector<Pair> &pairs, std::mt19937 &rng)
{
    Parity model(dimension);
    std::uniform_int_distribution<int> coefficient(0, kPrime - 1);
    for (const auto &pair : pairs)
    {
        std::array<std::vector<Parity::Term>, 2> terms;
        for (std::size_t side = 0; side < 2; ++side)
        {
            for (const auto &[index, value] : pair[side])
            {
                terms[side].push_back({index, Mint{value}});
            }
        }
        model.add_pair(terms[0], terms[1], Mint{coefficient(rng)});
    }
    return model.sample_size();
}

int main()
{
    Parity empty(0);
    empty.add_pair({}, {}, Mint{4});
    assert(empty.sample_size() == 0);

    Parity model(2);
    const std::array a{Parity::Term{0, Mint{2}}, Parity::Term{0, Mint{-1}}};
    const std::array b{Parity::Term{1, Mint{1}}};
    const std::array bad{Parity::Term{2, Mint{3}}};
    model.add_pair(a, b, Mint{0});
    assert(model.sample_size() == 0);
    model.add_pair(a, b, Mint{5});
    assert(model.sample_size() == 1);
    model.add_pair(b, a, Mint{5});
    assert(model.sample_size() == 0);
    for (bool left : {false, true})
    {
        bool caught = false;
        try
        {
            model.add_pair(left ? std::span(bad) : std::span(b),
                           left ? std::span(b) : std::span(bad), Mint{19});
        }
        catch (const std::out_of_range &)
        {
            caught = true;
        }
        assert(caught && model.sample_size() == 0);
    }

    std::mt19937 rng(231735);
    for (int trial = 0; trial < 320; ++trial)
    {
        const std::size_t dimension = 1 + rng() % 8;
        std::vector<Pair> pairs(rng() % 9);
        for (auto &pair : pairs)
        {
            for (auto &vector : pair)
            {
                const auto count = rng() % (2 * dimension + 1);
                for (std::size_t i = 0; i < count; ++i)
                {
                    vector.emplace_back(rng() % dimension, static_cast<int>(rng() % 13) - 6);
                }
            }
        }
        const auto expected = brute(dimension, pairs);
        const auto first = sample(dimension, pairs, rng), second = sample(dimension, pairs, rng);
        assert(first <= expected && second <= expected && std::max(first, second) == expected);
    }
}
