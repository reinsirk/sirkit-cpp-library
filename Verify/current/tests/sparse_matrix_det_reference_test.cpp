// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SPARSE_MATRIX_DETERMINANT_DEFINED
#include "../../../Math/Sparse_Matrix_Determinant.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Entry = sirkit::SparseDeterminantEntry;
constexpr std::uint32_t kMod = 998244353;

std::uint32_t leibniz(std::size_t n, const std::vector<Entry> &entries)
{
    std::vector<std::vector<std::uint64_t>> dense(n, std::vector<std::uint64_t>(n));
    for (const auto &[row, column, value] : entries)
    {
        dense[row][column] = (dense[row][column] + value) % kMod;
    }
    std::vector<std::size_t> permutation(n);
    std::iota(permutation.begin(), permutation.end(), std::size_t{0});
    std::uint64_t answer = 0;
    do
    {
        std::uint64_t product = 1;
        bool odd = false;
        for (std::size_t i = 0; i < n; ++i)
        {
            product = product * dense[i][permutation[i]] % kMod;
            for (std::size_t j = 0; j < i; ++j)
            {
                odd ^= permutation[j] > permutation[i];
            }
        }
        answer = (answer + (odd ? kMod - product : product)) % kMod;
    } while (std::ranges::next_permutation(permutation).found);
    return static_cast<std::uint32_t>(answer);
}

template <class Error, class Call>
void expect_exception(Call call)
{
    bool caught = false;
    try
    {
        call();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    using sirkit::sparse_determinant_mod998244353;
    assert(sparse_determinant_mod998244353(0, {}) == 1);
    assert(sparse_determinant_mod998244353(5, {}) == 0);
    assert(sparse_determinant_mod998244353(2, std::array{Entry{0, 0, 0}, Entry{1, 1, kMod}}) == 0);
    const std::vector<Entry> cancel{
        {0, 0, 2 * kMod - 1},
        {0, 0, 1},
        {1, 0, 1},
        {1, 1, kMod + 4},
    };
    assert(sparse_determinant_mod998244353(2, cancel) == 0);
    const std::vector<Entry> repeated_rows{{0, 0, 5}, {0, 1, 7}, {1, 0, 5}, {1, 1, 7}};
    assert(sparse_determinant_mod998244353(2, repeated_rows) == 0);

    // seed=0 の最初の試行で D*A が単位行列の定数倍になるように各成分を選ぶ。
    std::mt19937_64 seed_zero(0);
    const auto first = 1u + static_cast<std::uint32_t>(seed_zero() % (kMod - 1));
    const auto second = 1u + static_cast<std::uint32_t>(seed_zero() % (kMod - 1));
    const std::vector<Entry> retry{{0, 0, second}, {1, 1, first}};
    assert(!sparse_determinant_mod998244353(2, retry, 0, 0));
    assert(!sparse_determinant_mod998244353(2, retry, 0, 1));
    assert(sparse_determinant_mod998244353(2, retry, 0, 2) == leibniz(2, retry));

    std::mt19937 random(1042);
    for (int i = 0; i < 900; ++i)
    {
        const std::size_t n = 1 + random() % 7;
        const std::size_t k = random() % (2 * n * n + 1);
        std::vector<Entry> entries;
        for (std::size_t j = 0; j < k; ++j)
        {
            entries.push_back({random() % n, random() % n, static_cast<std::uint32_t>(random())});
        }
        if (i % 2 == 0)
        {
            std::shuffle(entries.begin(), entries.end(), random);
        }
        assert(sparse_determinant_mod998244353(n, entries, random()) == leibniz(n, entries));
    }

    constexpr std::size_t kMaxN = 3000, kMaxEntries = 10000;
    std::vector<Entry> triangular;
    std::uint64_t expected = 1;
    for (std::size_t i = 0; i < kMaxN; ++i)
    {
        const auto diagonal = static_cast<std::uint32_t>(i + 1);
        triangular.push_back({i, i, diagonal});
        expected = expected * diagonal % kMod;
    }
    for (std::size_t band = 1; triangular.size() < kMaxEntries; ++band)
    {
        for (std::size_t i = 0; i + band < kMaxN && triangular.size() < kMaxEntries; ++i)
        {
            triangular.push_back({i, i + band, 1});
        }
    }
    std::ranges::reverse(triangular);
    assert(sparse_determinant_mod998244353(kMaxN, triangular) == expected);

    auto rejection = [first_call = true]() mutable -> std::uint64_t
    {
        if (first_call)
        {
            first_call = false;
            return 0;
        }
        return 5;
    };
    assert(sirkit::detail::SparseDeterminantUniform(rejection, 3) == 2);
    assert(!sirkit::detail::SparseDeterminantFromRecurrence({}, 2, {1}));
    assert(sirkit::BerlekampMassey(std::vector<std::uint32_t>{}).empty());
    assert(sirkit::BerlekampMassey(std::vector<std::uint32_t>{1, 1, 1, 1}) == std::vector<std::uint32_t>{1});
    expect_exception<std::length_error>([]
                                        {
                                            (void)sirkit::detail::recurrence_size_plus_one(1, 1, "overflow");
                                        });
    expect_exception<std::length_error>([]
                                        {
                                            (void)sparse_determinant_mod998244353(std::numeric_limits<std::size_t>::max(), {});
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::invalid_argument>([]
                                            {
                                                (void)sparse_determinant_mod998244353(1, {}, 0, -1);
                                            });
    for (const auto entry : {Entry{1, 0, 1}, Entry{0, 1, 1}})
    {
        expect_exception<std::out_of_range>([&]
                                            {
                                                (void)sparse_determinant_mod998244353(1, std::array{entry});
                                            });
    }
#endif
}
