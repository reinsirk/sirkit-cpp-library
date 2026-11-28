// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Interval_Matrix_Rank.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Matrix = std::vector<std::vector<std::uint32_t>>;

// 独立な参照は各区間を最初から掃き出す。基底に行番号を付けない。
std::size_t GaussianRank(Matrix rows, std::uint32_t mod)
{
    const auto width = rows.front().size();
    for (auto &row : rows)
    {
        for (auto &value : row)
        {
            value %= mod;
        }
    }
    std::size_t rank = 0;
    for (std::size_t column = 0; column < width && rank < rows.size(); ++column)
    {
        std::size_t first = rank;
        while (first < rows.size() && rows[first][column] == 0)
        {
            ++first;
        }
        if (first == rows.size())
        {
            continue;
        }
        rows[rank].swap(rows[first]);
        const auto pivot = rows[rank][column];
        for (std::size_t i = rank + 1; i < rows.size(); ++i)
        {
            const auto weight = rows[i][column];
            for (std::size_t j = column; j < width; ++j)
            {
                const auto scaled = std::uint64_t{rows[i][j]} * pivot % mod;
                const auto subtracted = std::uint64_t{rows[rank][j]} * weight % mod;
                rows[i][j] = static_cast<std::uint32_t>((scaled + mod - subtracted) % mod);
            }
        }
        ++rank;
    }
    return rank;
}

template <std::uint32_t Mod>
void Check(const Matrix &rows, std::size_t width)
{
    for (std::size_t rank = 0; rank <= width + 1; ++rank)
    {
        const auto ends = sirkit::interval_matrix_rank_ends<Mod>(rows, rank);
        assert(ends.size() == rows.size());
        for (std::size_t left = 0; left < rows.size(); ++left)
        {
            std::optional<std::size_t> expected;
            if (rank == 0)
            {
                expected = left;
            }
            else
            {
                Matrix submatrix;
                for (std::size_t right = left; right < rows.size(); ++right)
                {
                    submatrix.push_back(rows[right]);
                    if (GaussianRank(submatrix, Mod) >= rank)
                    {
                        expected = right + 1;
                        break;
                    }
                }
            }
            assert(expected == ends[left]);
        }
    }
}

template <std::uint32_t Mod>
void RandomMatrices(std::mt19937 &random)
{
    for (int trial = 0; trial < 130; ++trial)
    {
        const auto height = random() % 9;
        const auto width = random() % 7;
        Matrix matrix(height, std::vector<std::uint32_t>(width));
        for (auto &row : matrix)
        {
            for (auto &value : row)
            {
                value = (trial % 3 == 0) ? random() % 7 : static_cast<std::uint32_t>(random());
            }
        }
        Check<Mod>(matrix, width);
    }
}
} // namespace

int main()
{
    const Matrix swaps{{0, 0, 1}, {1, 0, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1}, {0, 0, 0}};
    Check<998244353>(swaps, 3);
    Check<998244353>({{}, {}}, 0);
    Check<998244353>({}, 0);
    Check<4294967291>({{std::numeric_limits<std::uint32_t>::max(), 0}, {0, 4294967291u}, {1, 4}}, 2);

    bool ragged_rejected = false;
    try
    {
        (void)sirkit::interval_matrix_rank_ends(Matrix{{1}, {}, {2}}, 0);
    }
    catch (const std::invalid_argument &)
    {
        ragged_rejected = true;
    }
    assert(ragged_rejected);

    std::mt19937 random(399);
    RandomMatrices<998244353>(random);
    RandomMatrices<2>(random);
    RandomMatrices<7>(random);
    RandomMatrices<4294967291>(random);
}
