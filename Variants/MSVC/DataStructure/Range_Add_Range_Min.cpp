// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: DataStructure/Range_Add_Range_Min.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_add_range_min.md
//
// API一覧
// RangeAddRangeMin<T>(n/values): n個の0または初期配列から構築。O(n)時間・領域。
// add(l,r,d): 半開区間[l,r)にdを加える。O(log(n+1))。
// minimum(l,r): 区間の最小値。空区間はnullopt。O(log(n+1))。
// all_minimum(): 全体の最小値、空ならnullopt。size(): 要素数。O(1)。
// 添字は0始まり。Tは符号付き整数、内部の最小値・保留中の加算・計算の中間値もTの範囲内であること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_ADD_RANGE_MIN_DEFINED
#define SIRKIT_RANGE_ADD_RANGE_MIN_DEFINED

template <std::signed_integral T = std::int64_t>
class RangeAddRangeMin
{
    struct Node
    {
        T minimum = 0;
        T added = 0;
    };

public:
    explicit RangeAddRangeMin(std::size_t n = 0) : nodes_(Allocate(n))
    {
    }

    explicit RangeAddRangeMin(std::span<const T> values) : RangeAddRangeMin(values.size())
    {
        if (!values.empty())
        {
            Build(0, 0, values.size(), values);
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return (nodes_.size() + 1) / 2;
    }

    void add(std::size_t left, std::size_t right, T value)
    {
        CheckRange(left, right);
        if (left != right)
        {
            Add(0, 0, size(), left, right, value);
        }
    }

    [[nodiscard]] std::optional<T> minimum(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return std::nullopt;
        }
        return Min(0, 0, size(), left, right);
    }

    [[nodiscard]] std::optional<T> all_minimum() const noexcept
    {
        return nodes_.empty() ? std::optional<T>{} : std::optional<T>{nodes_[0].minimum};
    }

private:
    std::vector<Node> nodes_;

    static std::vector<Node> Allocate(std::size_t size)
    {
        if (size > std::vector<Node>().max_size() / 2)
        {
            throw std::length_error("RangeAddRangeMin: too many elements");
        }
        return std::vector<Node>(size == 0 ? 0 : 2 * size - 1);
    }

    void CheckRange([[maybe_unused]] std::size_t left, [[maybe_unused]] std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size())
        {
            throw std::out_of_range("RangeAddRangeMin: range");
        }
#endif
    }

    void Build(std::size_t p, std::size_t begin, std::size_t end, std::span<const T> values)
    {
        if (end - begin == 1)
        {
            nodes_[p].minimum = values[begin];
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto r = p + 2 * (mid - begin);
        Build(p + 1, begin, mid, values);
        Build(r, mid, end, values);
        nodes_[p].minimum = std::min(nodes_[p + 1].minimum, nodes_[r].minimum);
    }

    // 子の値は親の保留加算を含まない。区間加算と最小値が可換なので押し下げを省く。
    void Add(std::size_t p, std::size_t begin, std::size_t end,
             std::size_t left, std::size_t right, T value)
    {
        auto &node = nodes_[p];
        if (left <= begin && end <= right)
        {
            node.minimum += value;
            node.added += value;
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto r = p + 2 * (mid - begin);
        if (left < mid)
        {
            Add(p + 1, begin, mid, left, right, value);
        }
        if (right > mid)
        {
            Add(r, mid, end, left, right, value);
        }
        node.minimum = node.added + std::min(nodes_[p + 1].minimum, nodes_[r].minimum);
    }

    T Min(std::size_t p, std::size_t begin, std::size_t end, std::size_t left, std::size_t right) const
    {
        const auto &node = nodes_[p];
        if (left <= begin && end <= right)
        {
            return node.minimum;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto r = p + 2 * (mid - begin);
        if (right <= mid)
        {
            return node.added + Min(p + 1, begin, mid, left, right);
        }
        if (left >= mid)
        {
            return node.added + Min(r, mid, end, left, right);
        }
        return node.added + std::min(Min(p + 1, begin, mid, left, right),
                                     Min(r, mid, end, left, right));
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
