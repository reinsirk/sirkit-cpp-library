// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Manhattan_MST.cpp
// 参考: https://ei1333.github.io/library/graph/mst/manhattan-mst.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/manhattan_mst.md
//
// API一覧
// manhattan_mst(points): 格子点のマンハッタン距離による最小全域木。O(N log(N+1))時間・O(N)領域。
// pointsは(x,y)の列。同じ座標の頂点も可。各軸の絶対値はINT64_MAX/4以下。
// ManhattanMSTResult: costは距離の総和。edgesは元の0始まりの頂点番号の組。
// 頂点が0個か1個なら{0,{}}。総和がint64_tを超えるとstd::overflow_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MANHATTAN_MST_DEFINED
#define SIRKIT_MANHATTAN_MST_DEFINED

struct ManhattanMSTResult
{
    std::int64_t cost = 0;
    std::vector<std::pair<int, int>> edges;
};

namespace detail
{
inline int CheckedManhattanMSTSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("manhattan_mst: vertex count");
    }
    return static_cast<int>(size);
}

struct ManhattanRanks
{
    std::vector<int> position;
    int count = 0;
};

inline ManhattanRanks GetManhattanRanks(std::span<const std::pair<std::int64_t, std::int64_t>> points,
                                        std::vector<int> &order, bool use_x)
{
    const auto value = [&](int i)
    {
        return use_x ? points[i].first : points[i].second;
    };
    std::sort(order.begin(), order.end(), [&](int a, int b)
              {
                  return value(a) > value(b);
              });
    ManhattanRanks result{std::vector<int>(points.size()), 0};
    auto previous = value(order.front());
    for (int i : order)
    {
        const auto key = value(i);
        result.count += (key != previous);
        result.position[i] = result.count;
        previous = key;
    }
    ++result.count;
    return result;
}

// 1段上のビットは、下の段の64ビットに候補が残っていることを表す。
class ManhattanRankSet
{
    std::vector<std::vector<std::uint64_t>> words_;
    std::size_t end_;

public:
    explicit ManhattanRankSet(std::size_t size) : end_(size)
    {
        for (auto blocks = std::max(std::size_t{1}, (size + 63) / 64);; blocks = (blocks + 63) / 64)
        {
            words_.emplace_back(blocks);
            if (blocks == 1)
            {
                break;
            }
        }
    }

    std::size_t lower_bound(std::size_t index) const
    {
        std::size_t level = 0;
        while (true)
        {
            const auto block = index / 64;
            if (block >= words_[level].size())
            {
                return end_;
            }
            const auto bits = words_[level][block] & (~std::uint64_t{0} << (index % 64));
            if (bits != 0)
            {
                index = block * 64 + std::countr_zero(bits);
                break;
            }
            ++level;
            if (level == words_.size())
            {
                return end_;
            }
            index = block + 1;
        }
        while (level != 0)
        {
            --level;
            index = index * 64 + std::countr_zero(words_[level][index]);
        }
        return index;
    }

    void insert(std::size_t index)
    {
        for (auto &level : words_)
        {
            auto &word = level[index / 64];
            const bool nonempty = word != 0;
            word |= std::uint64_t{1} << (index % 64);
            if (nonempty)
            {
                break;
            }
            index /= 64;
        }
    }

    void erase(std::size_t index)
    {
        for (auto &level : words_)
        {
            auto &word = level[index / 64];
            word &= ~(std::uint64_t{1} << (index % 64));
            if (word != 0)
            {
                break;
            }
            index /= 64;
        }
    }
};
} // namespace detail

inline ManhattanMSTResult manhattan_mst(std::span<const std::pair<std::int64_t, std::int64_t>> points)
{
    const int n = detail::CheckedManhattanMSTSize(points.size());
    ManhattanMSTResult result;
    std::vector<std::pair<std::int64_t, std::int64_t>> transformed(points.begin(), points.end());
    if (n < 2)
    {
        return result;
    }
    struct Candidate
    {
        std::int64_t distance;
        int from;
        int to;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(points.size());
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    const auto x_rank = detail::GetManhattanRanks(points, order, true);
    const auto y_rank = detail::GetManhattanRanks(points, order, false);
    std::vector<int> pending(std::max(x_rank.count, y_rank.count));
    for (int direction = 0; direction < 4; ++direction)
    {
        // 座標を交換しても x+y は同じなので、直前の整列を再利用する。
        if (direction % 2 == 0)
        {
            std::sort(order.begin(), order.end(), [&](int lhs, int rhs)
                      {
                          const auto &[lx, ly] = transformed[lhs];
                          const auto &[rx, ry] = transformed[rhs];
                          return lx + ly < rx + ry;
                      });
        }
        // 4回の走査で第2座標は順にy、x、x、-y。-yの順位は逆順で再利用する。
        const auto &ranks = direction == 0 || direction == 3 ? y_rank : x_rank;
        const auto end = static_cast<std::size_t>(ranks.count);
        detail::ManhattanRankSet active(end);
        for (int current : order)
        {
            const auto &[x, y] = transformed[current];
            const auto rank = static_cast<std::size_t>(direction == 3
                                                           ? ranks.count - 1 - ranks.position[current]
                                                           : ranks.position[current]);
            for (auto next = active.lower_bound(rank); next != end; next = active.lower_bound(next))
            {
                const int previous = pending[next];
                const auto &[px, py] = transformed[previous];
                const auto dx = x - px, dy = y - py;
                if (dx < dy)
                {
                    break;
                }
                candidates.push_back({dx + dy, previous, current});
                active.erase(next);
            }
            pending[rank] = current;
            active.insert(rank);
        }
        for (auto &[x, y] : transformed)
        {
            if (direction % 2 == 0)
            {
                std::swap(x, y);
            }
            else
            {
                x = -x;
            }
        }
    }

    constexpr std::size_t kRadixThreshold = 1024;
    constexpr int kDigitBits = 8;
    constexpr std::size_t kBuckets = 1U << kDigitBits;
    constexpr auto kDigitMask = kBuckets - 1;
    if (candidates.size() < kRadixThreshold)
    {
        std::sort(candidates.begin(), candidates.end(), [](const Candidate &lhs, const Candidate &rhs)
                  {
                      return lhs.distance < rhs.distance;
                  });
    }
    else
    {
        std::uint64_t highest = 0;
        for (const auto &candidate : candidates)
        {
            highest = std::max(highest, static_cast<std::uint64_t>(candidate.distance));
        }
        if (highest != 0)
        {
            std::vector<Candidate> buffer(candidates.size());
            const int weight_bits = std::bit_width(highest);
            for (int shift = 0; shift < weight_bits; shift += kDigitBits)
            {
                std::array<std::size_t, kBuckets> count{};
                for (const auto &edge : candidates)
                {
                    ++count[(static_cast<std::uint64_t>(edge.distance) >> shift) & kDigitMask];
                }
                std::size_t seen = 0;
                for (auto &bucket : count)
                {
                    const auto frequency = bucket;
                    bucket = seen;
                    seen += frequency;
                }
                for (const auto &edge : candidates)
                {
                    buffer[count[(static_cast<std::uint64_t>(edge.distance) >> shift) & kDigitMask]++] = edge;
                }
                candidates.swap(buffer);
            }
        }
    }
    std::vector<int> parent(n, -1);
    const auto root = [&parent](int vertex)
    {
        int leader = vertex;
        while (parent[leader] >= 0)
        {
            leader = parent[leader];
        }
        while (vertex != leader)
        {
            const int next = parent[vertex];
            parent[vertex] = leader;
            vertex = next;
        }
        return leader;
    };
    result.edges.reserve(n - 1);
    const std::size_t needed = points.size() - 1;
    for (std::size_t i = 0; i < candidates.size() && result.edges.size() < needed; ++i)
    {
        const auto &[distance, from, to] = candidates[i];
        int left = root(from), right = root(to);
        if (left == right)
        {
            continue;
        }
        if (parent[left] > parent[right])
        {
            std::swap(left, right);
        }
        parent[left] += parent[right];
        parent[right] = left;
        if (distance > std::numeric_limits<std::int64_t>::max() - result.cost)
        {
            throw std::overflow_error("manhattan_mst: total weight");
        }
        result.cost += distance;
        result.edges.emplace_back(from, to);
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
