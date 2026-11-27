// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Algorithm/Static_Range_LIS.cpp
// 参考: P. Gawrychowski, E. Gorbachev, T. Kociumaka, Core-Sparse Monge Matrix Multiplication
// (https://doi.org/10.4230/LIPIcs.ESA.2025.74) の値による分割統治。行列積を含むコードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_range_lis.md
//
// API一覧
// StaticRangeLIS(sequence): 整数を値型とする長さ既知のランダムアクセス範囲を前処理する。
// length(left, right): 0始まり半開区間[left,right)の狭義最長増加部分列の長さ。空区間なら0。
// 負数、重複、空列も可。通常版で添字が範囲外、またはleft>rightならout_of_range。
// 前処理O(N log²(N+1))時間・O(N log(N+1))領域、問い合わせO(1+log(N+1))時間。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_RANGE_LIS_DEFINED
#define SIRKIT_STATIC_RANGE_LIS_DEFINED

namespace detail
{
using StaticLisIndex = std::size_t;

// 順列pを D(i,j)=#{x>=i : p[x]<j} とみなしたときのmin-plus積。
inline std::vector<StaticLisIndex> StaticLisUnitProduct(std::span<const StaticLisIndex> a,
                                                        std::span<const StaticLisIndex> b)
{
    using Index = StaticLisIndex;
    const auto n = a.size();
    if (n == 1)
    {
        return {0};
    }
    if (n == 2)
    {
        const auto first = a[0] | b[0];
        return {first, first ^ Index{1}};
    }
    const auto middle = n / 2;
    std::vector<Index> inverse_b(n), input_a(n), input_b(n), row_origin(n), column_origin(n), column_rank(n);
    std::array<Index, 2> row_cursor{0, middle}, column_cursor{0, middle};
    for (Index i = 0; i < n; ++i)
    {
        const bool upper = a[i] >= middle;
        const auto j = row_cursor[upper]++;
        row_origin[j] = i;
        input_a[j] = a[i] - upper * middle;
        inverse_b[b[i]] = i;
    }
    for (Index j = 0; j < n; ++j)
    {
        const bool upper = inverse_b[j] >= middle;
        const auto k = column_cursor[upper]++;
        column_origin[k] = j;
        column_rank[j] = k - upper * middle;
    }
    for (Index k = 0; k < n; ++k)
    {
        input_b[k] = column_rank[b[k]];
    }

    const auto left_a = std::span(input_a);
    const auto left_b = std::span(input_b);
    const auto low = StaticLisUnitProduct(left_a.first(middle), left_b.first(middle));
    const auto high = StaticLisUnitProduct(left_a.subspan(middle), left_b.subspan(middle));
    std::vector<Index> lifted_column(n), lifted_row(n);
    const auto lift = [&](std::span<const Index> product, Index offset)
    {
        for (Index i = 0; i < product.size(); ++i)
        {
            const auto row = row_origin[offset + i];
            const auto column = column_origin[offset + product[i]];
            lifted_column[row] = column;
            lifted_row[column] = row;
        }
    };
    lift(low, 0);
    lift(high, middle);

    // 2つの部分積の差は行・列を増やすごとに0か1だけ増える。
    // 各行で差が正になる最初の列を右から追い、積の行差分が1になる列を得る。
    std::vector<Index> result(n);
    Index delta = 0, col = n;
    for (Index i = 0; i < n; ++i)
    {
        const bool lower = a[i] < middle;
        delta += lower != (lifted_column[i] < col);
        if (delta != 0)
        {
            for (;;)
            {
                const bool drop = (inverse_b[col - 1] < middle) == (lifted_row[col - 1] > i);
                if (delta <= static_cast<Index>(drop))
                {
                    break;
                }
                delta -= drop;
                --col;
            }
        }
        const auto boundary = delta == 0 ? n : col - 1;
        result[i] = lower ? std::min(lifted_column[i], boundary) : std::max(lifted_column[i], boundary);
    }
    return result;
}

// 行または列の点が欠ける場合は、範囲外の行・列を足して順列に補完する。
inline std::vector<StaticLisIndex> StaticLisPartialProduct(std::span<const StaticLisIndex> a,
                                                           std::span<const StaticLisIndex> b)
{
    using Index = StaticLisIndex;
    const auto n = a.size();
    std::vector<bool> a_columns(n), b_columns(n);
    Index extra_rows = 0, extra_columns = 0;
    for (const auto c : a)
    {
        if (c == n)
        {
            ++extra_columns;
        }
        else
        {
            a_columns[c] = true;
        }
    }
    for (const auto c : b)
    {
        if (c == n)
        {
            ++extra_rows;
        }
        else
        {
            b_columns[c] = true;
        }
    }

    const auto extra = extra_rows + extra_columns;
    std::vector<Index> full_a, full_b;
    full_a.reserve(n + extra);
    full_b.reserve(n + extra);
    for (Index i = 0; i < extra_rows; ++i)
    {
        full_a.push_back(i);
    }
    for (Index j = 0; j < n; ++j)
    {
        if (!a_columns[j])
        {
            full_a.push_back(extra_rows + j);
        }
        if (!b_columns[j])
        {
            full_b.push_back(j);
        }
    }
    Index a_next = extra_rows + n, b_next = n;
    for (Index i = 0; i < n; ++i)
    {
        full_a.push_back(a[i] == n ? a_next++ : extra_rows + a[i]);
        full_b.push_back(b[i] == n ? b_next++ : b[i]);
    }
    for (Index i = 0; i < extra_columns; ++i)
    {
        full_b.push_back(n + extra_rows + i);
    }

    const auto product = StaticLisUnitProduct(full_a, full_b);
    std::vector<Index> result(n);
    for (Index i = 0; i < n; ++i)
    {
        result[i] = std::min(product[extra + i], n);
    }
    return result;
}

// LIS長の欠損 (r-l)-LIS(l,r) を、行・列に高々1点ずつの表現へ変換する。
inline std::vector<StaticLisIndex> StaticLisDeficits(std::span<const StaticLisIndex> permutation)
{
    using Index = StaticLisIndex;
    const auto n = permutation.size();
    if (n < 2)
    {
        return std::vector<Index>(n, n);
    }
    const auto middle = n / 2;
    std::array<std::vector<Index>, 2> groups, positions, lifted;
    for (Index i = 0; i < n; ++i)
    {
        const bool upper = permutation[i] >= middle;
        groups[upper].push_back(permutation[i] - upper * middle);
        positions[upper].push_back(i);
    }
    for (Index group = 0; group < 2; ++group)
    {
        const auto part = StaticLisDeficits(groups[group]);
        auto &result = lifted[group];
        result.resize(n);
        std::iota(result.begin(), result.end(), Index{0});
        for (Index i = 0; i < part.size(); ++i)
        {
            result[positions[group][i]] = part[i] == part.size() ? n : positions[group][part[i]];
        }
    }
    return StaticLisPartialProduct(lifted[0], lifted[1]);
}
} // namespace detail

class StaticRangeLIS
{
    using Index = std::size_t;

    struct Level
    {
        std::vector<Index> ones;
        Index zeros;
    };

public:
    template <class Sequence>
        requires std::ranges::random_access_range<const Sequence> && std::ranges::sized_range<const Sequence> &&
                 std::integral<std::ranges::range_value_t<const Sequence>>
    explicit StaticRangeLIS(const Sequence &values)
        : size_(static_cast<Index>(std::ranges::size(values)))
    {
        using Difference = std::ranges::range_difference_t<const Sequence>;
        using Symbol = std::ranges::range_value_t<const Sequence>;
        const auto first = std::ranges::begin(values);
        const auto value_at = [&](Index i)
        {
            return static_cast<Symbol>(first[static_cast<Difference>(i)]);
        };
        std::vector<Index> order(size_), permutation(size_);
        std::iota(order.begin(), order.end(), Index{0});
        // 同値を出現順の逆順に順位付けすると、同値2つを狭義増加列に選べない。
        std::ranges::sort(order, [&](Index a, Index b)
                          {
                              const auto av = value_at(a), bv = value_at(b);
                              return av == bv ? a > b : av < bv;
                          });
        for (Index i = 0; i < size_; ++i)
        {
            permutation[order[i]] = i;
        }
        BuildIndex(detail::StaticLisDeficits(permutation));
    }

    std::size_t length(std::size_t left, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size_)
        {
            throw std::out_of_range("StaticRangeLIS: invalid interval");
        }
#endif
        if (left == right)
        {
            return 0;
        }
        Index result = right - left;
        Index first = left, last = size_, bit = levels_.size();
        for (const auto &level : levels_)
        {
            --bit;
            const auto a = level.ones[first], b = level.ones[last];
            if (((right >> bit) & 1) != 0)
            {
                result -= (last - first) - (b - a);
                first = level.zeros + a;
                last = level.zeros + b;
            }
            else
            {
                first -= a;
                last -= b;
            }
        }
        return result;
    }

private:
    Index size_;
    std::vector<Level> levels_;

    void BuildIndex(std::vector<Index> values)
    {
        std::vector<Index> next(size_);
        const auto bits = std::bit_width(size_);
        levels_.reserve(bits);
        for (auto bit = bits; bit != 0;)
        {
            --bit;
            auto &level = levels_.emplace_back();
            level.ones.resize(size_ + 1);
            for (Index i = 0; i < size_; ++i)
            {
                level.ones[i + 1] = level.ones[i] + ((values[i] >> bit) & 1);
            }
            level.zeros = size_ - level.ones.back();
            Index zero = 0, one = level.zeros;
            for (const auto value : values)
            {
                auto &position = ((value >> bit) & 1) != 0 ? one : zero;
                next[position++] = value;
            }
            values.swap(next);
        }
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
