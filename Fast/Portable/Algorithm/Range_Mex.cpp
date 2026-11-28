// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Range_Mex.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_mex.md
//
// API一覧
// RangeMexQuery{left,right}: 0始まりの半開区間[left,right)。0<=left<=right<=N。
// range_mex(values,queries): 各区間のmex（含まれない最小の非負整数）を照会順で返す。空区間は0。
// 最長の区間長をKとしてO((N+Q)log(K+2))時間・O(N+Q)領域。
// subarray_mex_histogram(values): 全非空区間のmexの出現回数を0からNまで返す。O((N+1)log(N+2))時間・O(N+1)領域。
// 配列の要素は符号付き64ビット整数。負の値も許す。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_MEX_DEFINED
#define SIRKIT_RANGE_MEX_DEFINED

struct RangeMexQuery
{
    std::size_t left;
    std::size_t right;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t range_mex_base(std::size_t max_length)
{
    constexpr auto kMaxBase = std::size_t{1} << (std::numeric_limits<std::size_t>::digits - 2);
    if (max_length >= kMaxBase)
    {
        throw std::length_error("range_mex: interval too long");
    }
    return std::bit_ceil(max_length + 1);
}

inline std::uint64_t mex_subarray_count(std::size_t n)
{
    const auto odd = std::uint64_t{n | std::size_t{1}};
    const auto half_even = std::uint64_t{n / 2 + (n % 2)};
    if (half_even != 0 && odd > std::numeric_limits<std::uint64_t>::max() / half_even)
    {
        throw std::overflow_error("subarray_mex_histogram: interval count overflows");
    }
    return odd * half_even;
}

// 各左端で条件を満たす最小の右端は、単調非減少。同じ右端になる左端を区間ごとに保持する。
class MexRightBounds
{
public:
    MexRightBounds(std::size_t size, std::uint64_t count)
        : count_(count)
    {
        for (std::size_t pos = 0; pos <= size; ++pos)
        {
            bounds_.emplace_hint(bounds_.end(), pos, pos);
        }
    }

    void raise(std::size_t left, std::size_t height)
    {
        const auto head = std::prev(bounds_.upper_bound(left));
        if (head->second >= height)
        {
            return;
        }
        auto end = head;
        auto through = left;
        do
        {
            const auto next = std::next(end);
            count_ -= std::uint64_t{next->first - through} * (height - end->second);
            through = next->first;
            end = next;
        } while (end->second < height); // 左端がheight以降なら元の右端もheight以上になる。
        bounds_.erase(head->first == left ? head : std::next(head), end);
        bounds_.emplace_hint(end, left, height);
    }

    [[nodiscard]] std::uint64_t count() const
    {
        return count_;
    }

private:
    std::map<std::size_t, std::size_t> bounds_;
    std::uint64_t count_;
};
} // namespace detail

inline std::vector<std::size_t> range_mex(std::span<const std::int64_t> values,
                                          std::span<const RangeMexQuery> queries)
{
    std::size_t max_length = 0;
    std::size_t last_right = 0;
    for (const auto &[left, right] : queries)
    {
        if (left < right)
        {
            max_length = std::max(max_length, right - left);
            last_right = std::max(last_right, right);
        }
    }
    const auto no_query = queries.size();
    std::vector<std::size_t> result(no_query);
    if (max_length == 0)
    {
        return result;
    }
    std::vector<std::size_t> head(last_right, no_query), next_query(no_query);
    for (std::size_t id = 0; id < no_query; ++id)
    {
        const auto &[left, right] = queries[id];
        if (left < right)
        {
            next_query[id] = std::exchange(head[right - 1], id);
        }
    }
    const auto base = detail::range_mex_base(max_length);
    std::vector<std::size_t> minimum(base * 2);
    for (std::size_t i = 0; i < last_right; ++i)
    {
        const auto value = values[i];
        if (value >= 0 && static_cast<std::uint64_t>(value) <= max_length)
        {
            auto node = base + static_cast<std::size_t>(value);
            minimum[node] = i + 1; // 1始まりの位置と未出現を表す0を区別する。
            while (node > 1)
            {
                node /= 2;
                minimum[node] = std::min(minimum[2 * node], minimum[2 * node + 1]);
            }
        }
        for (auto id = head[i]; id != no_query; id = next_query[id])
        {
            auto node = std::size_t{1};
            while (node < base)
            {
                node *= 2;
                if (minimum[node] > queries[id].left)
                {
                    ++node;
                }
            }
            result[id] = node - base;
        }
    }
    return result;
}

inline std::vector<std::uint64_t> subarray_mex_histogram(std::span<const std::int64_t> values)
{
    const auto n = values.size();
    auto count = detail::mex_subarray_count(n);
    std::vector<std::uint64_t> result(n + 1);
    std::vector<std::vector<std::size_t>> occurrence(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto value = values[i];
        if (value >= 0 && static_cast<std::uint64_t>(value) < n)
        {
            occurrence[static_cast<std::size_t>(value)].push_back(i);
        }
    }
    detail::MexRightBounds right_bounds(n, count);
    for (std::size_t value = 0; value < n && count != 0; ++value)
    {
        std::size_t start = 0;
        for (const auto pos : occurrence[value])
        {
            right_bounds.raise(start, pos);
            start = pos + 1;
        }
        if (start < n)
        {
            right_bounds.raise(start, n);
        }
        const auto remaining = right_bounds.count();
        result[value] = count - remaining;
        count = remaining;
    }
    result[n] = count;
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
