// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Neighborhood_Assignment.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/neighborhood_assignment.md
//
// API一覧
// NeighborhoodAssignment<T>(initial, edges, threshold=0): 無向グラフに初期値を設定。
//   多重辺・自己辺を扱う。threshold=0ならfloor(sqrt(2m))+1。O(n+m)時間・領域。
// size(): 頂点数。O(1)。
// get(v): 頂点vの現在値。しきい値Bに対しO(1+m/B)。
// assign(v, value): 頂点vの値を上書き。O(1)。
// assign_neighbors(v, value, include_self=false): vと隣接する頂点を一括で上書き。
//   include_self=trueならvにも代入。O(1+B)。値のコピーはO(1)と仮定。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NEIGHBORHOOD_ASSIGNMENT_DEFINED
#define SIRKIT_NEIGHBORHOOD_ASSIGNMENT_DEFINED

namespace detail
{
inline int neighborhood_assignment_size(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("NeighborhoodAssignment: too many vertices");
    }
    return static_cast<int>(size);
}

inline std::uint64_t next_neighborhood_assignment_stamp(std::uint64_t stamp)
{
    if (stamp == std::numeric_limits<std::uint64_t>::max())
    {
        throw std::overflow_error("NeighborhoodAssignment: too many assignments");
    }
    return stamp + 1;
}
} // namespace detail

template <class T>
class NeighborhoodAssignment
{
    static constexpr bool kStoreDirect =
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>;
    using Storage = std::conditional_t<kStoreDirect, T, std::shared_ptr<const T>>;

    struct Assignment
    {
        std::uint64_t stamp;
        Storage value;
    };

public:
    NeighborhoodAssignment(std::span<const T> initial, std::span<const std::pair<int, int>> edges,
                           std::size_t threshold = 0)
        : adjacent_(detail::neighborhood_assignment_size(initial.size())),
          heavy_neighbors_(initial.size()), pending_index_(initial.size(), -1)
    {
        direct_.reserve(initial.size());
        for (const T &value : initial)
        {
            direct_.push_back({0, store(value)});
        }
        for (const auto &[u, v] : edges)
        {
            check_vertex(u);
            check_vertex(v);
            adjacent_[u].push_back(v);
            adjacent_[v].push_back(u);
        }
        if (threshold == 0)
        {
            threshold = static_cast<std::size_t>(std::sqrt(2.0 * static_cast<double>(edges.size()))) + 1;
        }

        std::vector<int> seen(initial.size(), -1);
        for (int v = 0; v < size(); ++v)
        {
            if (adjacent_[v].size() <= threshold)
            {
                continue;
            }
            pending_index_[v] = static_cast<int>(pending_.size());
            pending_.push_back(direct_[v]);
            for (const int u : adjacent_[v])
            {
                if (seen[u] != v)
                {
                    seen[u] = v;
                    heavy_neighbors_[u].push_back(pending_index_[v]);
                }
            }
            std::vector<int>{}.swap(adjacent_[v]);
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(direct_.size());
    }

    [[nodiscard]] T get(int v) const
    {
        check_vertex(v);
        const Assignment *newest = &direct_[v];
        for (const int index : heavy_neighbors_[v])
        {
            const auto &pending = pending_[index];
            if (pending.stamp > newest->stamp)
            {
                newest = &pending;
            }
        }
        if constexpr (kStoreDirect)
        {
            return newest->value;
        }
        else
        {
            return *newest->value;
        }
    }

    void assign(int v, const T &value)
    {
        check_vertex(v);
        const auto change = new_assignment(value);
        direct_[v] = change;
    }

    void assign_neighbors(int v, const T &value, bool include_self = false)
    {
        check_vertex(v);
        const auto change = new_assignment(value);
        if (include_self)
        {
            direct_[v] = change;
        }
        const int index = pending_index_[v];
        if (index != -1)
        {
            pending_[index] = change;
        }
        else
        {
            for (const int neighbor : adjacent_[v])
            {
                direct_[neighbor] = change;
            }
        }
    }

private:
    void check_vertex(int v) const
    {
        static_cast<void>(v);
    }

    static Storage store(const T &value)
    {
        if constexpr (kStoreDirect)
        {
            return value;
        }
        else
        {
            return std::make_shared<const T>(value);
        }
    }

    Assignment new_assignment(const T &value)
    {
        const Assignment change{detail::next_neighborhood_assignment_stamp(stamp_), store(value)};
        stamp_ = change.stamp;
        return change;
    }

    std::vector<std::vector<int>> adjacent_;
    std::vector<std::vector<int>> heavy_neighbors_;
    std::vector<int> pending_index_;
    std::vector<Assignment> direct_;
    std::vector<Assignment> pending_;
    std::uint64_t stamp_ = 0;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
