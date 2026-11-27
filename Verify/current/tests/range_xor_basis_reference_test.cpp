// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Range_Xor_Basis.cpp"
#include <cassert>
#include <random>
#include <set>

int main()
{
    using sirkit::range_xor_queries;
    using sirkit::RangeXorQuery;
    std::mt19937_64 random(49724);
    assert(range_xor_queries({}, {}).empty());
    const std::uint64_t high = std::uint64_t{1} << 63;
    const auto full = range_xor_queries(std::vector<std::uint64_t>{high, high, 3, 0},
                                        std::vector<RangeXorQuery>{{0, 2, high}, {1, 4, 3}, {3, 3, high}, {4, 4, 0}, {2, 3, high}});
    assert(full[0].can_make && full[0].rank == 1 && full[0].maximum == high);
    assert(full[1].can_make && full[1].rank == 2 && full[1].maximum == (high | 3));
    assert(!full[2].can_make && full[2].rank == 0 && full[2].maximum == high);
    assert(full[3].can_make && full[3].rank == 0 && full[3].maximum == 0);
    assert(!full[4].can_make && full[4].rank == 1 && full[4].maximum == (high | 3));
    for (unsigned trial = 0; trial < 700; ++trial)
    {
        const std::size_t n = random() % 13;
        std::vector<std::uint64_t> values(n);
        const auto mask = (trial % 2 == 0) ? 511 : ~std::uint64_t{0};
        for (auto &x : values)
        {
            x = random() & mask;
        }
        std::vector<RangeXorQuery> queries;
        for (unsigned i = 0; i < 32; ++i)
        {
            std::size_t left = random() % (n + 1), right = random() % (n + 1);
            if (left > right)
            {
                std::swap(left, right);
            }
            queries.push_back({left, right, random() & mask});
        }
        const auto result = range_xor_queries(values, queries);
        for (std::size_t q = 0; q < queries.size(); ++q)
        {
            const auto [left, right, x] = queries[q];
            std::vector<std::uint64_t> attainable{0};
            for (std::size_t i = left; i < right; ++i)
            {
                const auto old_size = attainable.size();
                for (std::size_t j = 0; j < old_size; ++j)
                {
                    attainable.push_back(attainable[j] ^ values[i]);
                }
            }
            const std::set<std::uint64_t> distinct(attainable.begin(), attainable.end());
            std::uint64_t biggest = 0;
            for (const auto v : distinct)
            {
                biggest = std::max(biggest, x ^ v);
            }
            assert(result[q].can_make == distinct.contains(x));
            assert(result[q].maximum == biggest);
            assert((std::size_t{1} << result[q].rank) == distinct.size());
        }
    }
    std::vector<std::uint64_t> basis(64);
    for (unsigned i = 0; i < 64; ++i)
    {
        basis[i] = std::uint64_t{1} << i;
    }
    const auto complete = range_xor_queries(basis, std::vector<RangeXorQuery>{{0, 64, ~high}})[0];
    assert(complete.rank == 64 && complete.can_make && complete.maximum == ~std::uint64_t{0});
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto q : {RangeXorQuery{1, 0, 0}, RangeXorQuery{0, 1, 0}})
    {
        try
        {
            range_xor_queries({}, std::span<const RangeXorQuery>(&q, 1));
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
