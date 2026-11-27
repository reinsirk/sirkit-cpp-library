// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Algorithm/Common_Interval_Decomposition.cpp
// 参考: I. Belghiti, M. Habib, A general method for common intervals
// (https://arxiv.org/abs/1309.7141) の商と分解木。区間探索を含むコードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/common_interval_decomposition.md
//
// API一覧
// common_interval_decomposition(permutation): 0..N-1の順列の強区間木を、親が先の順序で返す。O(N log(N+1))時間、O(N)領域。
// CommonIntervalNode: 位置[left,right)、値[value_begin,value_end)、親parent（根は-1）、種類kind。
// 順列の部分区間の値が連続し、他の連続値区間と包含・互いに素であるとき強区間という。
// CommonIntervalKind: leaf（1要素）、increasing/decreasing（子の値区間が順/逆順）、prime（それ以外）。
// 空列は空。通常版で範囲外や重複値はinvalid_argument。節の添字はint。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMMON_INTERVAL_DECOMPOSITION_DEFINED
#define SIRKIT_COMMON_INTERVAL_DECOMPOSITION_DEFINED

enum class CommonIntervalKind
{
    leaf,
    increasing,
    decreasing,
    prime,
};

struct CommonIntervalNode
{
    int left;
    int right;
    int value_begin;
    int value_end;
    int parent;
    CommonIntervalKind kind;
};

namespace detail
{
inline int CommonIntervalSize(std::size_t count)
{
    constexpr auto kMaximum = static_cast<std::size_t>(std::numeric_limits<int>::max() / 2) + 1;
    if (count > kMaximum)
    {
        throw std::length_error("common_interval_decomposition: node indices do not fit in int");
    }
    return static_cast<int>(count);
}

class CommonIntervalScoreTree
{
    struct Entry
    {
        std::int64_t minimum;
        std::int64_t pending;
    };

public:
    CommonIntervalScoreTree(int n, std::int64_t inactive)
        : leaves_(static_cast<int>(std::bit_ceil(static_cast<unsigned>(n)))),
          scores_(2 * static_cast<std::size_t>(leaves_), Entry{inactive, 0})
    {
    }

    void add(int first, int last, std::int64_t delta)
    {
        if (first < last)
        {
            Add(1, 0, leaves_, first, last, delta);
        }
    }

    void set(int position, std::int64_t value)
    {
        Set(1, 0, leaves_, position, value);
    }

    int last_zero_before(int limit)
    {
        return LastZero(1, 0, leaves_, limit);
    }

private:
    int leaves_;
    std::vector<Entry> scores_;

    void Apply(int node, std::int64_t delta)
    {
        scores_[node].minimum += delta;
        scores_[node].pending += delta;
    }

    void Push(int node)
    {
        if (const auto delta = scores_[node].pending; delta != 0)
        {
            Apply(2 * node, delta);
            Apply(2 * node + 1, delta);
            scores_[node].pending = 0;
        }
    }

    void Pull(int node)
    {
        scores_[node].minimum = std::min(scores_[2 * node].minimum, scores_[2 * node + 1].minimum);
    }

    void Add(int node, int low, int high, int first, int last, std::int64_t delta)
    {
        if (first <= low && high <= last)
        {
            Apply(node, delta);
            return;
        }
        Push(node);
        const int middle = low + (high - low) / 2;
        if (first < middle)
        {
            Add(2 * node, low, middle, first, last, delta);
        }
        if (middle < last)
        {
            Add(2 * node + 1, middle, high, first, last, delta);
        }
        Pull(node);
    }

    void Set(int node, int low, int high, int position, std::int64_t value)
    {
        if (high - low == 1)
        {
            scores_[node] = {value, 0};
            return;
        }
        Push(node);
        const int middle = low + (high - low) / 2;
        if (position < middle)
        {
            Set(2 * node, low, middle, position, value);
        }
        else
        {
            Set(2 * node + 1, middle, high, position, value);
        }
        Pull(node);
    }

    int LastZero(int node, int low, int high, int limit)
    {
        if (low >= limit || scores_[node].minimum != 0)
        {
            return -1;
        }
        if (high - low == 1)
        {
            return low;
        }
        Push(node);
        const int middle = low + (high - low) / 2;
        const int right = LastZero(2 * node + 1, middle, high, limit);
        return right >= 0 ? right : LastZero(2 * node, low, middle, limit);
    }
};

struct CommonIntervalRawNode
{
    CommonIntervalNode interval;
    int first_child = -1;
    int next_sibling = -1;
};

inline std::vector<CommonIntervalNode> NormalizeCommonIntervals(
    const std::vector<CommonIntervalRawNode> &raw, int root)
{
    struct Task
    {
        int node;
        int parent;
    };
    std::vector<Task> pending{{root, -1}};
    std::vector<CommonIntervalNode> result;
    result.reserve(raw.size());
    while (!pending.empty())
    {
        auto [index, parent] = pending.back();
        pending.pop_back();
        const auto &node = raw[index];
        if (node.next_sibling >= 0)
        {
            pending.push_back({node.next_sibling, parent});
        }
        const auto kind = node.interval.kind;
        const bool monotone = kind == CommonIntervalKind::increasing || kind == CommonIntervalKind::decreasing;
        // 同じ向きの線形の親子を縮約する。残る節が強区間になる。
        if (!monotone || parent < 0 || result[parent].kind != kind)
        {
            const int id = static_cast<int>(result.size());
            result.push_back(node.interval);
            result.back().parent = parent;
            parent = id;
        }
        if (node.first_child >= 0)
        {
            pending.push_back({node.first_child, parent});
        }
    }
    return result;
}
} // namespace detail

inline std::vector<CommonIntervalNode> common_interval_decomposition(std::span<const int> permutation)
{
    if (permutation.empty())
    {
        return {};
    }
    const int n = detail::CommonIntervalSize(permutation.size());
#ifndef SKIP_RUNTIME_CHECKS
    {
        std::vector<bool> seen(n);
        for (const auto x : permutation)
        {
            if (x < 0 || x >= n || seen[x])
            {
                throw std::invalid_argument("common_interval_decomposition: expected a permutation of 0..N-1");
            }
            seen[x] = true;
        }
    }
#endif

    const auto inactive = static_cast<std::int64_t>(n) + 1;
    detail::CommonIntervalScoreTree score(n, inactive);
    std::vector<detail::CommonIntervalRawNode> raw;
    raw.reserve(n + (n - 1));
    std::vector<int> forest;
    forest.reserve(n);

    struct Extremum
    {
        int first;
        int value;
    };
    std::vector<Extremum> maxima, minima;
    for (int r = 0; r < n; ++r)
    {
        // 左端lの値は max(P[l..r])-min(P[l..r])-(r-l)。0が連続値区間。
        score.add(0, r, -1);
        const auto replace_extrema = [&](std::vector<Extremum> &ranges, bool maximum)
        {
            int l = r;
            while (!ranges.empty() &&
                   (maximum ? ranges.back().value < permutation[r] : ranges.back().value > permutation[r]))
            {
                const auto [first, before] = ranges.back();
                ranges.pop_back();
                const auto delta = maximum ? permutation[r] - before : before - permutation[r];
                score.add(first, l, delta);
                l = first;
            }
            ranges.push_back({l, permutation[r]});
        };
        replace_extrema(maxima, true);
        replace_extrema(minima, false);
        score.set(r, 0);

        forest.push_back(static_cast<int>(raw.size()));
        raw.push_back({{r, r + 1, permutation[r], permutation[r] + 1, -1, CommonIntervalKind::leaf}});
        for (;;)
        {
            const int l = score.last_zero_before(raw[forest.back()].interval.left);
            if (l < 0)
            {
                break;
            }

            // 現在の森の根を2個以上まとめる、最短の連続値区間を選ぶ。
            int first_child = -1, count = 0, value_begin = n, value_end = 0;
            for (;;)
            {
                const int child = forest.back();
                forest.pop_back();
                const auto &interval = raw[child].interval;
                raw[child].next_sibling = first_child;
                first_child = child;
                ++count;
                value_begin = std::min(value_begin, interval.value_begin);
                value_end = std::max(value_end, interval.value_end);
                if (interval.left == l)
                {
                    break;
                }
                score.set(interval.left, inactive);
            }

            // 2子より多い場合は、子の並びに非自明な連続値区間がない。
            auto kind = CommonIntervalKind::prime;
            if (count == 2)
            {
                kind = raw[first_child].interval.value_begin == value_begin
                           ? CommonIntervalKind::increasing
                           : CommonIntervalKind::decreasing;
            }
            forest.push_back(static_cast<int>(raw.size()));
            raw.push_back({{l, r + 1, value_begin, value_end, -1, kind}, first_child});
        }
    }
    return detail::NormalizeCommonIntervals(raw, forest.back());
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
